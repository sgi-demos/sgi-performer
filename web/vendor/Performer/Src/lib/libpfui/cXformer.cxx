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
 * cXformer.C
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
#include <Performer/pfui.h>
#include "pfiXformer.h"

PFUIDLLEXPORT pfType*
pfiGetXformerClassType(void)
{
    return pfiXformer::getClassType();    
}

PFUIDLLEXPORT pfiXformer *
pfiNewXformer(void *arena)
{
    return new(arena) pfiXformer;
}

PFUIDLLEXPORT void 
pfiXformerModel(pfiXformer* xf, int _index, pfiInputXform* _model)
{
     if (xf == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE, 
	    "pfiXformerModel -- null pfiXformer*");
	return;
    }
    xf->setModel(_index, _model);
}

PFUIDLLEXPORT void 
pfiSelectXformerModel(pfiXformer *xf, int _which)
{ 
    if (xf == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE, 
	    "pfiSelectXformerModel -- null pfiXformer*");
	return;
    }
    xf->selectModel(_which);
}

PFUIDLLEXPORT pfiInputXform*  
pfiGetXformerCurModel(pfiXformer *xf)
{ 
    if (xf == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE, 
	    "pfiGetXformerCurModel -- null pfiXformer*");
	return NULL;
    }
    return xf->getCurModel();
}

PFUIDLLEXPORT int
pfiGetXformerCurModelIndex(pfiXformer *xf)
{ 
    if (xf == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE, 
	    "pfiGetXformerCurModelIndex -- null pfiXformer*");
	return -1;
    }
    return xf->getCurModelIndex();
}

PFUIDLLEXPORT void 
pfiRemoveXformerModel(pfiXformer* xf, pfiInputXform* _model)
{
     if (xf == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE, 
	    "pfiRemoveXformerModel -- null pfiXformer*");
	return;
    }
    xf->removeModel(_model);
}

PFUIDLLEXPORT void 
pfiRemoveXformerModelIndex(pfiXformer* xf, int _index)
{
     if (xf == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE, 
	    "pfiRemoveXformerModelIndex -- null pfiXformer*");
	return;
    }
    xf->removeModelIndex(_index);
}

PFUIDLLEXPORT void
pfiStopXformer(pfiXformer *xf)
{
    if (xf == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE, 
	    "pfiStopXformer -- null pfiXformer*");
	return;
    }
    xf->stop();
}

PFUIDLLEXPORT void
pfiResetXformer(pfiXformer *xf)
{
    if (xf == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE, 
	    "pfiResetXformer -- null pfiXformer*");
	return;
    }
    xf->reset();
}

PFUIDLLEXPORT void
pfiResetXformerPosition(pfiXformer *xf)
{
    if (xf == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE, 
	    "pfiResetXformerPosition -- null pfiXformer*");
	return;
    }
    xf->resetPosition();
}

PFUIDLLEXPORT void
pfiCenterXformer(pfiXformer *xf)
{
    if (xf == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE, 
	    "pfiCenterXformer -- null pfiXformer*");
	return;
    }
    xf->center();
}


PFUIDLLEXPORT void
pfiXformerAutoInput(pfiXformer *xf, pfChannel *_chan, pfuMouse *_mouse, 
    pfuEventStream *_events)
{
    if (xf == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE, 
	    "pfiXformerAutoInput -- null pfiXformer*");
	return;
    }

    xf->setAutoInput(_chan, _mouse, _events);
}

PFUIDLLEXPORT void
pfiGetXformerAutoInput(pfiXformer *xf, pfChannel **_chan, pfuMouse **_mouse, 
    pfuEventStream **_events)
{
    if (xf == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE, 
	    "pfiXformerAutoInput -- null pfiXformer*");
	return;
    }

    xf->getAutoInput(_chan, _mouse, _events);
}

PFUIDLLEXPORT void
pfiXformerMat(pfiXformer *xf, pfMatrix& _mat)
{
    if (xf == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE, 
	    "pfiXformerMat -- null pfiXformer*");
	return;
    }
    xf->setMat(_mat);
}


PFUIDLLEXPORT void
pfiGetXformerMat(pfiXformer *xf, pfMatrix& _mat)
{
    if (xf == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE, 
	    "pfiGetXformerMat -- null pfiXformer*");
	return;
    }

    xf->getMat(_mat);
}

PFUIDLLEXPORT void
pfiXformerModelMat(pfiXformer *xf, pfMatrix& _mat)
{
    if (xf == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE, 
	    "pfiXformerModelMat -- null pfiXformer*");
	return;
    }
    xf->setModelMat(_mat);
}


PFUIDLLEXPORT void
pfiGetXformerModelMat(pfiXformer *xf, pfMatrix& _mat)
{
    if (xf == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE, 
	    "pfiGetXformerModelMat -- null pfiXformer*");
	return;
    }

    xf->getModelMat(_mat);
}


PFUIDLLEXPORT void
pfiXformerCoord(pfiXformer *xf, pfCoord *_coord)
{
    if (xf == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE, 
	    "pfiXformerCoord -- null pfiXformer*");
	return;
    }
    xf->setCoord(_coord);
}


PFUIDLLEXPORT void
pfiGetXformerCoord(pfiXformer *xf, pfCoord *_coord)
{
    if (xf == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE, 
	    "pfiGetXformerCoord -- null pfiXformer*");
	return;
    }
    xf->getCoord(_coord); 
}

PFUIDLLEXPORT void
pfiXformerResetCoord(pfiXformer *xf, pfCoord *_resetCoord)
{
    if (xf == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE, 
	    "pfiXformerCoord -- null pfiXformer*");
	return;
    }
    xf->setResetCoord(_resetCoord);
}


PFUIDLLEXPORT void
pfiGetXformerResetCoord(pfiXformer *xf, pfCoord *_resetCoord)
{
    if (xf == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE, 
	    "pfiGetXformerCoord -- null pfiXformer*");
	return;
    }
    xf->getResetCoord(_resetCoord); 
}

PFUIDLLEXPORT void
pfiXformerNode(pfiXformer *xf, pfNode *_node)
{
   if (xf == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE, 
	    "pfiXformerNode -- null pfiXformer*");
	return;
    } 
    xf->setNode(_node);
}

PFUIDLLEXPORT pfNode *
pfiGetXformerNode(pfiXformer *xf)
{
   if (xf == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE, 
	    "pfiGetXformerNode -- null pfiXformer*");
	return NULL;
    } 
    return xf->getNode();
}

PFUIDLLEXPORT void
pfiXformerAutoPosition(pfiXformer *xf, pfChannel *_chan, pfDCS *_dcs)
{
   if (xf == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE, 
	    "pfiXformerAutoPosition -- null pfiXformer*");
	return;
    } 
    xf->setAutoPosition(_chan, _dcs);
}

PFUIDLLEXPORT void
pfiGetXformerAutoPosition(pfiXformer *xf, pfChannel **_chan, pfDCS **_dcs)
{
   if (xf == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE, 
	    "pfiGetXformerAutoPosition -- null pfiXformer*");
	return;
    } 
    xf->getAutoPosition(_chan, _dcs);
}


PFUIDLLEXPORT void
pfiXformerLimits(pfiXformer *xf, float _maxSpeed, float _angularVel, 
    float _accel, pfBox *_dbLimits)
{
    if (xf == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE, 
	    "pfiXformerLimits -- null pfiXformer*");
	return;
    }
    xf->setMotionLimits(_maxSpeed, _angularVel, _accel);
    xf->setDBLimits(_dbLimits);
}

PFUIDLLEXPORT void
pfiGetXformerLimits(pfiXformer *xf, float *_maxSpeed, float *_angularVel, 
    float *_accel, pfBox *_dbLimits)
{
    if (xf == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE, 
	    "pfiGetXformerLimits -- null pfiXformer*");
	return;
    }
    xf->getMotionLimits(_maxSpeed, _angularVel, _accel);
    xf->getDBLimits(_dbLimits);
}

PFUIDLLEXPORT void
pfiXformerCollision(pfiXformer *xf, int _mode, float _val, pfNode *_node)
{
    if (xf == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE, 
	    "pfiXformerCollision -- null pfiXformer*");
	return;
    }
    xf->setCollision(_mode, _val, _node);
}

PFUIDLLEXPORT void
pfiEnableXformerCollision(pfiXformer *xf)
{
    if (xf == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE, 
	    "pfiEnableXformerCollision -- null pfiXformer*");
	return;
    }
    xf->enableCollision();
}

PFUIDLLEXPORT void
pfiDisableXformerCollision(pfiXformer *xf)
{
    if (xf == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE, 
	    "pfiDisableXformerCollision -- null pfiXformer*");
	return;
    }
    xf->disableCollision();
}

PFUIDLLEXPORT int
pfiGetXformerCollisionEnable(pfiXformer *xf)
{
    if (xf == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE, 
	    "pfiGetXformerCollisionEnable -- null pfiXformer*");
	return -1;
    }
    return xf->getCollisionEnable();
}


PFUIDLLEXPORT int
pfiGetXformerCollisionStatus(pfiXformer *xf)
{
    if (xf == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE, 
	    "pfiGetXformerCollisionStatus -- null pfiXformer*");
	return -1;
    }
    return xf->getCollisionStatus();
}


PFUIDLLEXPORT void
pfiUpdateXformer(pfiXformer *xf)
{
    if (xf == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE, 
	    "pfiUpdateXformer -- null pfiXformer*");
	return;
    }
    xf->update();
}

PFUIDLLEXPORT int
pfiCollideXformer(pfiXformer *xf)
{
    if (xf == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE, 
	    "pfiCollideXformer -- null pfiXformer*");
	return NULL;
    }
    return xf->collide();
}


/* TDF Xformer Routines */


PFUIDLLEXPORT pfType*
pfiGetTDFXformerClassType(void)
{
    return pfiTDFXformer::getClassType();    
}

PFUIDLLEXPORT pfiTDFXformer *
pfiNewTDFXformer(void *arena)
{
    return new(arena) pfiTDFXformer;
}

PFUIDLLEXPORT pfiXformer *
pfiCreateTDFXformer(pfiInputXformTrackball *_tb,
				pfiInputXformDrive *_drive,
				pfiInputXformFly *_fly,
		                pfiInputXformSpheric *_spheric, void *arena)
{ 
    return new(arena) pfiTDFXformer(_tb, _drive, _fly, _spheric);
}

PFUIDLLEXPORT void
pfiTDFXformerFastClickTime(pfiTDFXformer *xf, float _clickTime)
{
   if (xf == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE, 
	    "pfiXformerHeight -- null pfiXformer*");
	return;
    } 
    xf->setFastClickTime(_clickTime);
}

PFUIDLLEXPORT float
pfiGetTDFXformerFastClickTime(pfiTDFXformer *xf)
{
   if (xf == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE, 
	    "pfiGetTDFXformerFastClickTime -- null pfiXformer*");
	return -1;
    } 
    return xf->getFastClickTime();
}

PFUIDLLEXPORT void 
pfiTDFXformerTrackball(pfiTDFXformer *xf, pfiInputXformTrackball *_tb)
{ 
    if (xf == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE, 
	    "pfiXformerTrackball -- null pfiXformer*");
	return;
    }
    xf->setTrackball(_tb);
}

PFUIDLLEXPORT pfiInputXformTrackball  *
pfiGetTDFXformerTrackball(pfiTDFXformer *xf)
{ 
    if (xf == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE, 
	    "pfiGetXformerTrackball -- null pfiXformer*");
	return NULL;
    }
    return xf->getTrackball();
}

PFUIDLLEXPORT void 
pfiTDFXformerDrive(pfiTDFXformer *xf, pfiInputXformDrive *_tb)
{ 
    if (xf == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE, 
	    "pfiXformerTrackball -- null pfiXformer*");
	return;
    }
    xf->setDrive(_tb);
}

PFUIDLLEXPORT pfiInputXformDrive *
pfiGetTDFXformerDrive(pfiTDFXformer *xf)
{ 
    if (xf == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE, 
	    "pfiGetXformerTrackball -- null pfiXformer*");
	return NULL;
    }
    return xf->getDrive();
}

PFUIDLLEXPORT void pfiTDFXformerFly(pfiTDFXformer *xf, pfiInputXformFly *_fly)
{ 
    if (xf == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE, 
	    "pfiXformerFly -- null pfiXformer*");
	return;
    }
    xf->setFly(_fly);
}

PFUIDLLEXPORT pfiInputXformFly * 
pfiGetTDFXformerFly(pfiTDFXformer *xf)
{ 
    if (xf == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE, 
	    "pfiGetXformerFly -- null pfiXformer*");
	return NULL;
    }
    return xf->getFly();
}

PFUIDLLEXPORT void pfiTDFXformerSpheric(pfiTDFXformer *xf, pfiInputXformSpheric *_spheric)
{
    if (xf == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE,
	    "pfiXformerSpheric -- null pfiXformer*");
	return;
    }
    xf->setSpheric(_spheric);
}

PFUIDLLEXPORT pfiInputXformSpheric *
pfiGetTDFXformerSpheric(pfiTDFXformer *xf)
{
    if (xf == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE,
	    "pfiGetXformerSpheric -- null pfiXformer*");
	return NULL;
    }
    return xf->getSpheric();
}
