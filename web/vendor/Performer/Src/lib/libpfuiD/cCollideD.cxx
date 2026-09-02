/*
 * WARNING! DO NOT EDIT!
 * cCollideD.C automatically generated from (../libpfui/cCollide.C)
 */
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
#include <Performer/pfuiD.h>

#include "pfiCollideD.h"



PFUIDLLEXPORT pfType*
pfiCollideDClassType(void)
{
    return pfiCollideD::getClassType();    
}

PFUIDLLEXPORT pfiCollideD*
pfiNewCollideD(void *arena)
{ 
    return new(arena) pfiCollideD;
}

PFUIDLLEXPORT void
pfiEnableCollideD(pfiCollideD *collide)
{ 
    if (collide == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE, 
	    "pfiEnableCollideD -- null pfiCollideD*");
	return;
    }
    collide->enable();
}

PFUIDLLEXPORT void
pfiDisableCollideD(pfiCollideD *collide)
{ 
    if (collide == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE, 
	    "pfiEnableCollideD -- null pfiCollideD*");
	return;
    }
    collide->disable();
}

PFUIDLLEXPORT int
pfiGetCollideDEnable(pfiCollideD *collide)
{ 
    if (collide == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE, 
	    "pfiGetCollideDEnable -- null pfiCollideD*");
	return -1;
    }
    return collide->getEnable();
}

PFUIDLLEXPORT void
pfiCollideDMode(pfiCollideD *collide, int _mode, int _val)
{ 
    if (collide == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE, 
	    "pfiCollideDMode -- null pfiCollideD*");
	return;
    }
    collide->setMode(_mode, _val);
}

PFUIDLLEXPORT int
pfiGetCollideDMode(pfiCollideD *collide, int _mode)
{ 
    if (collide == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE, 
	    "pfiGetCollideDMode -- null pfiCollideD*");
	return -1;
    }
    return collide->getMode(_mode);
}

PFUIDLLEXPORT void
pfiCollideDStatus(pfiCollideD *collide, int _status)
{ 
    if (collide == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE, 
	    "pfiCollideDStatus -- null pfiCollideD*");
	return;
    }
    collide->setStatus(_status);
}

PFUIDLLEXPORT int
pfiGetCollideDStatus(pfiCollideD *collide)
{ 
    if (collide == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE, 
	    "pfiGetCollideDStatus -- null pfiCollideD*");
	return -1;
    }
    return collide->getStatus();
}

PFUIDLLEXPORT void
pfiCollideDDist(pfiCollideD *collide, double _dist)
{ 
    if (collide == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE, 
	    "pfiCollideDDist -- null pfiCollideD*");
	return;
    }
    collide->setDist(_dist);
}

PFUIDLLEXPORT double
pfiGetCollideDDist(pfiCollideD *collide)
{ 
    if (collide == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE, 
	    "pfiGetCollideDDist -- null pfiCollideD*");
	return -1;
    }
    return collide->getDist();
}

PFUIDLLEXPORT void
pfiCollideDHeightAboveGrnd(pfiCollideD *collide, double _dist)
{ 
    if (collide == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE, 
	    "pfiCollideDHeightAboveGrnd -- null pfiCollideD*");
	return;
    }
    collide->setHeightAboveGrnd(_dist);
}

PFUIDLLEXPORT double
pfiGetCollideDHeightAboveGrnd(pfiCollideD *collide)
{ 
    if (collide == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE, 
	    "pfiGetCollideDHeightAboveGrnd -- null pfiCollideD*");
	return -1;
    }
    return collide->getHeightAboveGrnd();
}

PFUIDLLEXPORT void
pfiCollideDGroundNode(pfiCollideD *collide, pfNode* _ground)
{ 
    if (collide == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE, 
	    "pfiCollideDGroundNode -- null pfiCollideD*");
	return;
    }
    collide->setGroundNode(_ground);
}

PFUIDLLEXPORT pfNode*
pfiGetCollideDGroundNode(pfiCollideD *collide)
{ 
    if (collide == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE, 
	    "pfiGetCollideDGroundNode -- null pfiCollideD*");
	return NULL;
    }
    return collide->getGroundNode();
}

PFUIDLLEXPORT void
pfiCollideDObjNode(pfiCollideD *collide, pfNode* _db)
{ 
    if (collide == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE, 
	    "pfiCollideDObjNode -- null pfiCollideD*");
	return;
    }
    collide->setObjNode(_db);
}

PFUIDLLEXPORT pfNode*
pfiGetCollideDObjNode(pfiCollideD *collide)
{ 
    if (collide == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE, 
	    "pfiGetCollideDObjNode -- null pfiCollideD*");
	return NULL;
    }
    return collide->getObjNode();
}

PFUIDLLEXPORT void
pfiCollideDXformCoordCurMotionParams(pfiCollideD *collide, 
		pfCoordd* _pos, pfCoordd* _prevPos, double _speed)
{ 
    if (collide == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE, 
	    "pfiCollideDXformCoordCurMotionParams -- null pfiCollideD*");
	return;
    }
    collide->setCurMotionParams(_pos, _prevPos, _speed);
}

PFUIDLLEXPORT void
pfiGetCollideDCurMotionParams(pfiCollideD *collide, 
		pfCoordd* _pos, pfCoordd* _prevPos, double *_speed)
{ 
    if (collide == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE, 
	    "pfiGetCollideDCurMotionParams -- null pfiCollideD*");
	return;
    }
    collide->getCurMotionParams(_pos, _prevPos, _speed);
}

PFUIDLLEXPORT void
pfiCollideDFunc(pfiCollideD *collide, 
			    pfiCollideDFuncType _func, void *_data)
{ 
    if (collide == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE, 
	    "pfiCollideDFunc -- null pfiCollideD*");
	return;
    }
    collide->setCollisionFuncD(_func, _data);
}

PFUIDLLEXPORT void
pfiGetCollideDFunc(pfiCollideD *collide, 
			    pfiCollideDFuncType *_func, void **_data)
{ 
    if (collide == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE, 
	    "pfiGetCollideDFunc -- null pfiCollideD*");
	return;
    }
    collide->getCollisionFuncD(_func, _data);
}

PFUIDLLEXPORT int
pfiUpdateCollideD(pfiCollideD *collide)
{ 
    if (collide == NULL)
    {
	pfNotify(PFNFY_WARN, PFNFY_USAGE, 
	    "pfiUpdateCollideD -- null pfiCollideD*");
	return -1;
    }
    return collide->update();
}

