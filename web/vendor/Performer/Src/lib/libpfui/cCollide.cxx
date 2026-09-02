/*
 * Copyright 1992, 1995, Silicon Graphics, Inc.
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
 * file: cCollide.C
 * -----------------
 * 
 * $Revision: 1.8 $
 * $Date: 2002/07/24 01:09:01 $
 * 
 * IRIS Performer Utility Collide Class Interface Routines
 */



#include <Performer/pf.h>
#include <Performer/pr/pfMemory.h>
#include <Performer/pr/pfLinMath.h>
#include <Performer/pfui.h>

#include "pfiCollide.h"



PFUIDLLEXPORT pfType*
pfiCollideClassType(void)
{
    return pfiCollide::getClassType();    
}

PFUIDLLEXPORT pfiCollide*
pfiNewCollide(void *arena)
{ 
    return new(arena) pfiCollide;
}

PFUIDLLEXPORT void
pfiEnableCollide(pfiCollide *collide)
{ 
    if (collide == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE, 
	    "pfiEnableCollide -- null pfiCollide*");
	return;
    }
    collide->enable();
}

PFUIDLLEXPORT void
pfiDisableCollide(pfiCollide *collide)
{ 
    if (collide == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE, 
	    "pfiEnableCollide -- null pfiCollide*");
	return;
    }
    collide->disable();
}

PFUIDLLEXPORT int
pfiGetCollideEnable(pfiCollide *collide)
{ 
    if (collide == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE, 
	    "pfiGetCollideEnable -- null pfiCollide*");
	return -1;
    }
    return collide->getEnable();
}

PFUIDLLEXPORT void
pfiCollideMode(pfiCollide *collide, int _mode, int _val)
{ 
    if (collide == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE, 
	    "pfiCollideMode -- null pfiCollide*");
	return;
    }
    collide->setMode(_mode, _val);
}

PFUIDLLEXPORT int
pfiGetCollideMode(pfiCollide *collide, int _mode)
{ 
    if (collide == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE, 
	    "pfiGetCollideMode -- null pfiCollide*");
	return -1;
    }
    return collide->getMode(_mode);
}

PFUIDLLEXPORT void
pfiCollideStatus(pfiCollide *collide, int _status)
{ 
    if (collide == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE, 
	    "pfiCollideStatus -- null pfiCollide*");
	return;
    }
    collide->setStatus(_status);
}

PFUIDLLEXPORT int
pfiGetCollideStatus(pfiCollide *collide)
{ 
    if (collide == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE, 
	    "pfiGetCollideStatus -- null pfiCollide*");
	return -1;
    }
    return collide->getStatus();
}

PFUIDLLEXPORT void
pfiCollideDist(pfiCollide *collide, float _dist)
{ 
    if (collide == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE, 
	    "pfiCollideDist -- null pfiCollide*");
	return;
    }
    collide->setDist(_dist);
}

PFUIDLLEXPORT float
pfiGetCollideDist(pfiCollide *collide)
{ 
    if (collide == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE, 
	    "pfiGetCollideDist -- null pfiCollide*");
	return -1;
    }
    return collide->getDist();
}

PFUIDLLEXPORT void
pfiCollideHeightAboveGrnd(pfiCollide *collide, float _dist)
{ 
    if (collide == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE, 
	    "pfiCollideHeightAboveGrnd -- null pfiCollide*");
	return;
    }
    collide->setHeightAboveGrnd(_dist);
}

PFUIDLLEXPORT float
pfiGetCollideHeightAboveGrnd(pfiCollide *collide)
{ 
    if (collide == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE, 
	    "pfiGetCollideHeightAboveGrnd -- null pfiCollide*");
	return -1;
    }
    return collide->getHeightAboveGrnd();
}

PFUIDLLEXPORT void
pfiCollideGroundNode(pfiCollide *collide, pfNode* _ground)
{ 
    if (collide == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE, 
	    "pfiCollideGroundNode -- null pfiCollide*");
	return;
    }
    collide->setGroundNode(_ground);
}

PFUIDLLEXPORT pfNode*
pfiGetCollideGroundNode(pfiCollide *collide)
{ 
    if (collide == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE, 
	    "pfiGetCollideGroundNode -- null pfiCollide*");
	return NULL;
    }
    return collide->getGroundNode();
}

PFUIDLLEXPORT void
pfiCollideObjNode(pfiCollide *collide, pfNode* _db)
{ 
    if (collide == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE, 
	    "pfiCollideObjNode -- null pfiCollide*");
	return;
    }
    collide->setObjNode(_db);
}

PFUIDLLEXPORT pfNode*
pfiGetCollideObjNode(pfiCollide *collide)
{ 
    if (collide == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE, 
	    "pfiGetCollideObjNode -- null pfiCollide*");
	return NULL;
    }
    return collide->getObjNode();
}

PFUIDLLEXPORT void
pfiCollideXformCoordCurMotionParams(pfiCollide *collide, 
		pfCoord* _pos, pfCoord* _prevPos, float _speed)
{ 
    if (collide == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE, 
	    "pfiCollideXformCoordCurMotionParams -- null pfiCollide*");
	return;
    }
    collide->setCurMotionParams(_pos, _prevPos, _speed);
}

PFUIDLLEXPORT void
pfiGetCollideCurMotionParams(pfiCollide *collide, 
		pfCoord* _pos, pfCoord* _prevPos, float *_speed)
{ 
    if (collide == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE, 
	    "pfiGetCollideCurMotionParams -- null pfiCollide*");
	return;
    }
    collide->getCurMotionParams(_pos, _prevPos, _speed);
}

PFUIDLLEXPORT void
pfiCollideFunc(pfiCollide *collide, 
			    pfiCollideFuncType _func, void *_data)
{ 
    if (collide == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE, 
	    "pfiCollideFunc -- null pfiCollide*");
	return;
    }
    collide->setCollisionFunc(_func, _data);
}

PFUIDLLEXPORT void
pfiGetCollideFunc(pfiCollide *collide, 
			    pfiCollideFuncType *_func, void **_data)
{ 
    if (collide == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE, 
	    "pfiGetCollideFunc -- null pfiCollide*");
	return;
    }
    collide->getCollisionFunc(_func, _data);
}

PFUIDLLEXPORT int
pfiUpdateCollide(pfiCollide *collide)
{ 
    if (collide == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE, 
	    "pfiUpdateCollide -- null pfiCollide*");
	return -1;
    }
    return collide->update();
}

