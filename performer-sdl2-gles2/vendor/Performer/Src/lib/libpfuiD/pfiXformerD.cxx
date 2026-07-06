/*
 * WARNING! DO NOT EDIT!
 * pfiXformerD.C automatically generated from (../libpfui/pfiXformer.C)
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
 * pfiXformerD.C
 * --------------
 * $Revision: 1.93 $
 * $Date: 2002/12/09 04:24:43 $
 */

#include <math.h>
#ifndef WIN32
#include <X11/X.h>
#else
#include <windows.h>
#endif

#define PF_C_API 1 // enable C API also 

#include <Performer/pf.h>
#include <Performer/pf/pfDoubleDCS.h>
#include <Performer/pf/pfNode.h>
#include <Performer/pr/pfMemory.h>


#define _PFI_XFORMER_C_
#include <Performer/pfuiD.h>
#include <Performer/pfuiD/pfiXformerD.h>
#include <Performer/pr/pfGeoSet.h>


PFUIDLLEXPORT pfType *pfiXformerD::classType = NULL;

void
pfiXformerD::init()
{
    if (classType == NULL)
    {
	pfMemory::init();
	classType = 
	    new pfType(pfiXformerD::getClassType(), "pfiXformerD");
    }
}


pfiXformerD::pfiXformerD(void)
{
    void *arena		= pfMemory::getArena(this);
    
    setType(classType);
    xDefaultModelIndex = 0;
    xInputDChan             = NULL;
    xPosChan		   = NULL;
    xNode		   = NULL;
    xPosDCS		   = NULL;
    xMouse                 = NULL;
    xIx			   = NULL;
    xModelList = new(arena) pfList;
    xCollide = pfiNewCollideD(arena);
}

pfiXformerD::~pfiXformerD(void)
{
}

void
pfiXformerD::reset(void)
{
    int i, numModels;
    pfiInputXformD	*icx;
    pfiInputXformD::reset();
    xCurModelIndex            = xDefaultModelIndex;
    if ((xDefaultModelIndex > -1) && (xModelList->getNum() > xDefaultModelIndex))
	xIx = (pfiInputXformD*) xModelList->get(xDefaultModelIndex);
    else
	xIx = NULL;
    numModels = xModelList->getNum();
    for (i=0; i < numModels; i++)
    {
	icx = (pfiInputXformD*) xModelList->get(i);
	if (icx)
	    icx->reset();
    }

    xIx->getMotionCoordD(&(xMotionCoordD));
    xCollide->reset();
}

void
pfiXformerD::resetPosition(void)
{
    xIx->resetPosition();
    xIx->getMotionCoordD(&(xMotionCoordD));
    xCollide->setCurMotionParams(&xMotionCoordD.pos, &xMotionCoordD.prevPos, 
		xMotionCoordD.speed);
    if (xIx->isOfType(pfiInputXformDDrive::getClassType()))
    {
	double dh = ((pfiInputXformDDrive *)xIx)->getDriveHeight();
	setCollision(PFUCOLLIDE_GROUND, dh, getNode());
    }
    
    if (xPosChan)
     	pfChanViewMatD(xPosChan, xMotionCoordD.mat);

    if (xPosDCS)
	pfDoubleDCSMat(xPosDCS, pfIdentMat4d);
}

void
pfiXformerD::center(void)
{
    pfCoordd center;

    PFCOPY_VEC3(center.xyz, xDBSphere.center);
    PFSET_VEC3(center.hpr, 0.0f, 0.0f, 0.0f);
    xIx->setCoord(&center);
    xIx->getMotionCoordD(&(xMotionCoordD));
    xCollide->setCurMotionParams(&xMotionCoordD.pos, &xMotionCoordD.prevPos, 
		xMotionCoordD.speed);
    if (xIx->isOfType(pfiInputXformDDrive::getClassType()))
    {
	double dh = ((pfiInputXformDDrive *)xIx)->getDriveHeight();
	setCollision(PFUCOLLIDE_GROUND, dh, getNode());
    }
		
    if (xPosChan)
	pfChanViewMatD(xPosChan, xMotionCoordD.mat);
}

void
pfiXformerD::setModel(int _which, pfiInputXformD *model)
{
    xModelList->set(_which, model) ;
}

void
pfiXformerD::selectModel(int _which)
{
    if (_which > xModelList->getNum())
    {
	pfNotify(PFNFY_WARN, PFNFY_RESOURCE, 
	    "pfiSelectXformerDModel - index %d is more than number of models (%d)", 
	    _which, xModelList->getNum());
	return;
    }
    stop();
    xIx = (pfiInputXformD *) xModelList->get(_which) ;
}

pfiInputXformD *
pfiXformerD::getModel(int _which)
{
    if (_which > xModelList->getNum())
    {
	pfNotify(PFNFY_WARN, PFNFY_RESOURCE, 
	    "pfiGetXformerDModel - index %d is more than number of models (%d)", 
	    _which, xModelList->getNum());
	return NULL;
    }
    return (pfiInputXformD*) xModelList->get(_which) ;
}

void	
pfiXformerD::removeModelIndex(int _which)
{
    if (_which > xModelList->getNum())
    {
	pfNotify(PFNFY_WARN, PFNFY_RESOURCE, 
	    "pfiRemoveXformerDModel - index %d is more than number of models (%d)", 
	    _which, xModelList->getNum());
	return;
    }
    xModelList->set(_which, NULL);
}

void	
pfiXformerD::removeModel(pfiInputXformD *_icx)
{
    int i, numModels, found = 0;
    pfiInputXformD	*icx;
    numModels = xModelList->getNum();
    for (i=0; i < numModels; i++)
    {
	icx = (pfiInputXformD*) xModelList->get(i);
	if (icx == _icx)
	{
	    xModelList->set(i, NULL);
	    found = 1;
	    break;
	}
    }
    if (!found)
	pfNotify(PFNFY_WARN, PFNFY_RESOURCE, 
	    "pfiRemoveXformerDModel - couldn't find model 0x%p to remove.", _icx);
}

void
pfiXformerD::stop(void)
{
    pfiInputXformD::stop();
    if (xIx)
    {
	xIx->stop();
	xIx->getMotionCoordD(&(xMotionCoordD));
    }
    xCollide->setCurMotionParams(&xMotionCoordD.pos, &xMotionCoordD.prevPos, 
		xMotionCoordD.speed);
}

void
pfiXformerD::update(void)
{    

    // process input events
    if (xInputD)
	xInputD->processEvents();
	
    if (xIx)
    {
	/* update motion model with info from collide */
	xIx->setMotionCoordD(&(xMotionCoordD));
	xIx->update();
    }
    
    xIx->getMotionCoordD(&(xMotionCoordD));
}

void
pfiXformerD::setAutoInputD(pfChannel *_chan, pfuMouse *_mouse, 
    pfuEventStream *_events)
{
    xMouse  = _mouse;
    xInputDChan   = _chan;
    if (xInputD)
	xInputD->setEventStream(_events);
    pfuCollisionChan(xInputDChan);
}

void
pfiXformerD::getAutoInputD(pfChannel **_chan, pfuMouse **_mouse, 
    pfuEventStream **_events)
{
    *_mouse = xMouse;
    *_chan = xInputDChan;
    if (xInputD)
	*_events = xInputD->getEventStream();
    else
	*_events = NULL;
}

void
pfiXformerD::setMat(pfMatrix4d& _mat)
{
    int i, numModels;
    pfiInputXformD	*icx;
    
    numModels = xModelList->getNum();
    for (i=0; i < numModels; i++)
    {
	icx = (pfiInputXformD*) xModelList->get(i);
	if (icx)
	    icx->setMat(_mat);
    }
    if (xIx)
	xIx->getMotionCoordD(&(xMotionCoordD));
    xCollide->setCurMotionParams(&xMotionCoordD.pos, &xMotionCoordD.prevPos, 
		xMotionCoordD.speed);
}

void
pfiXformerD::getMat(pfMatrix4d& _mat)
{
    PFCOPY_MAT(_mat,  xMotionCoordD.mat);
}



void
pfiXformerD::setModelMat(pfMatrix4d& _mat)
{	    
    if (xIx)
    {
	xIx->setMat(_mat);
	xIx->getMotionCoordD(&(xMotionCoordD));
    }
    xCollide->setCurMotionParams(&xMotionCoordD.pos, &xMotionCoordD.prevPos, 
		xMotionCoordD.speed);
}

void
pfiXformerD::getModelMat(pfMatrix4d& _mat)
{
    if (xIx)
	xIx->getMat(_mat);
}


void
pfiXformerD::setResetCoord(pfCoordd *_coord) 
{
    int i, numModels;
    pfiInputXformD	*icx;
    
    xMotionCoordD.startPos = *_coord;
    numModels = xModelList->getNum();
    for (i=0; i < numModels; i++)
    {
	icx = (pfiInputXformD*) xModelList->get(i);
	if (icx)
	    icx->setResetCoord(_coord);
    }
	
    xUserSetMask |= _PFIX_USET_RESET_POS;
}

void
pfiXformerD::getResetCoord(pfCoordd *_coord) 
{
    *_coord = xMotionCoordD.startPos;
}
    
void
pfiXformerD::setCoord(pfCoordd *_coord)
{
    int i, numModels;
    pfiInputXformD	*icx;
    
    if (_coord == NULL)
	return;
  
    numModels = xModelList->getNum();
    for (i=0; i < numModels; i++)
    {
	icx = (pfiInputXformD*) xModelList->get(i);
	if (icx)
	    icx->setCoord(_coord);
    }
    if (xIx)
	xIx->getMotionCoordD(&(xMotionCoordD));
	
    xCollide->setCurMotionParams(&xMotionCoordD.pos, &xMotionCoordD.prevPos, 
		xMotionCoordD.speed);
}

void
pfiXformerD::setMotionLimits(double _maxSpeed, double _angularVel, double _accel)
{
    int i, numModels;
    pfiInputXformD	*icx;
    
    pfiInputXformD::setMotionLimits(_maxSpeed, _angularVel, _accel);
    numModels = xModelList->getNum();
    for (i=0; i < numModels; i++)
    {
	icx = (pfiInputXformD*) xModelList->get(i);
	if (icx)
	    icx->setMotionLimits(_maxSpeed, _angularVel, _accel);
    }
}

void
pfiXformerD::setAutoPosition(pfChannel *_chan, pfDoubleDCS *_dcs)
{ 
    if (_dcs)
	_dcs->ref(); 
    if (xPosDCS)
	xPosDCS->unref();
    xPosChan = _chan; 
    xPosDCS = _dcs; 
}

void
pfiXformerD::setBSphere(pfSphere *_sphere)
{
    int i, numModels;
    pfiInputXformD	*icx;
    
    xDBSphere = *_sphere;
    xUserSetMask |= _PFIX_USET_BSPHERE;
    
    numModels = xModelList->getNum();
    for (i=0; i < numModels; i++)
    {
	icx = (pfiInputXformD*) xModelList->get(i);
	if (icx)
	    icx->setBSphere(_sphere);
    }
}

void
pfiXformerD::setNode(pfNode *_node)
{ 
    pfBox bbox;
    pfSphere sph;
    int bound;
    double bmax=0.0f;
    
    if (_node)
	_node->ref(); 
    if (xNode)
	xNode->unref();
    xNode = _node; 
    
    // See if the user has set a bounding sphere on the node
    // handle has fixed sphere on the XformerD
    bound = _node->getBound(&sph);

    if (bound == PFBOUND_STATIC) // fixes bounding sphere
	setBSphere(&sph);
	
    if (!(xUserSetMask & _PFIX_USET_DB_LIMITS))
    {
	if (sph.radius > 0.0f)
	{
	    pfuTravCalcBBox(xNode, &bbox);
	    bmax = PF_MAX2(bbox.max[0] - bbox.min[0], 
		    PF_MAX2(bbox.max[1] - bbox.min[1], 
			bbox.max[2] - bbox.min[2]));
	}
	// see if the bbox is bogus and if so, use huge bbox
	if ((sph.radius < 0.0f) || (bmax < (sph.radius * 0.01f)) && (bmax < 1e-6f))
	{
	    // if box was bogus but sphere is ok, use sphere radius and center
	    if (sph.radius > 0.0f)
	    {
		bmax = sph.radius;
		setBSphere(&sph);
	    }
	    else
		sph.radius = PFI_BIGDB;
	    PFSET_VEC3(bbox.min, sph.center[0] - sph.radius, 
		sph.center[1] - sph.radius, 
		sph.center[2] - sph.radius);
	    PFSET_VEC3(bbox.max,  sph.center[0] + sph.radius, 
		sph.center[1] + sph.radius, 
		sph.center[2] + sph.radius);
	}
	_setDBLimits(&bbox);
    }
}

void
pfiXformerD::_setDBLimits(pfBox *_dbLimits)
{
    int i, numModels;
    pfiInputXformD	*icx;
    
    if (!_dbLimits)
	return;

    pfiInputXformD::_setDBLimits(_dbLimits); // udpates the BSphere
    numModels = xModelList->getNum();
    for (i=0; i < numModels; i++)
    {
	icx = (pfiInputXformD*) xModelList->get(i);
	if (icx)
	    icx->_setDBLimits(_dbLimits);
    }
}

void
pfiXformerD::setCollision(int _mode, double _val, pfNode *_node)
{
    int cmode;
	
    cmode = pfiGetCollideDMode(xCollide, PFIC_TARGET);
    switch (_mode) 
    {
    case PFUCOLLIDE_GROUND:
	xCollide->setHeightAboveGrnd( _val);
	xCollide->setGroundNode(_node);

	if (_node != NULL && _val > 0.0f)
	    xCollide->setMode(PFIC_TARGET, cmode | PFIC_TARGET_GROUND);
	else
	    xCollide->setMode(PFIC_TARGET, cmode & ~PFIC_TARGET_GROUND);
	break;

    case PFUCOLLIDE_OBJECT:
	xCollide->setDist(_val);
	xCollide->setObjNode(_node);

	if (_node != NULL && _val > 0.0f)
	    xCollide->setMode(PFIC_TARGET, cmode | PFIC_TARGET_OBJ);
	else
	    xCollide->setMode(PFIC_TARGET, cmode & ~PFIC_TARGET_OBJ);
	break;
    }
}
int
pfiXformerD::collide(void)
{
    int ret;
    
    pfuCollisionChan(xInputDChan);
    
    /* update the collider with new position and motion from motion model */
    xCollide->setCurMotionParams(&xMotionCoordD.pos, &xMotionCoordD.prevPos, 
		xMotionCoordD.speed);
    ret = xCollide->update();
    
    /* If collisions are enabled,
     * get prev position back from collider (may be running in 
     * a separate asynch process but partial updates are probably ok).
     */
    if (xCollide->getMode(PFIC_TARGET))
    {
	xCollide->getCurMotionParams(&xMotionCoordD.pos, &xMotionCoordD.prevPos, 
		&xMotionCoordD.speed);
	xMotionCoordD.makeMat();
    }
    return ret;
}

void
pfiXformerD::enableCollision(void)
{
    if (xCollide)
	xCollide->enable();
}

void
pfiXformerD::disableCollision(void)
{
    if (xCollide)
	xCollide->disable();
}

int
pfiXformerD::getCollisionEnable(void)
{
    if (xCollide)
	return xCollide->getEnable();
    else
	return 0;
}

int
pfiXformerD::getCollisionStatus(void)
{
    if (xCollide)
	return xCollide->getStatus();
    else
	return 0;
}

/*
 * pfiHaveFastMouseClickD() detects fast mouse clicks 
 */
int
pfiHaveFastMouseClickD(pfuMouse *mouse, int button, double msecs)
{
    int dist;
    double etime=0.0f;
    static int prevClick;
    static double prevClickStamp=-1.0f;
    
    if (msecs < 0.0f)
	return 0;
    
    /* msecs between click and release */
    if ((prevClick == button) && (prevClickStamp > 0.0f))
	etime = mouse->releaseTime[button] - prevClickStamp;
	
    /* if no middle click  or release last frame, no  fast stop */
    if (!((mouse->click & button) || (mouse->release & button)))
    {
	if ((!(mouse->flags & button)) ||
	    (etime > msecs))
	{
	    prevClick = 0;
	    prevClickStamp = -1.0f;
	}
	return 0;
    }
    
    /* msecs between click and release */
    if ((mouse->releaseTime[button] - mouse->clickTime[button]) > 1e-5f)
	etime = mouse->releaseTime[button] - mouse->clickTime[button];
	
    /* max pixels moved in x or y */
    dist = PF_MAX2(PF_ABS((mouse->releasePos[button][0] - mouse->clickPos[button][0])), 
		    PF_ABS((mouse->releasePos[button][1] - mouse->clickPos[button][1])));


    /* middle mouse was pushed & released in last frame and total distance
     * moved and elapsed was very small then have fast click
     */
    if ((mouse->release & button) && 
	    (mouse->click & button) &&
	    (etime >= 0.0f))
    {
	if ((dist < 10) && (etime < msecs))
	{
	    prevClick = 0;
	    prevClickStamp = -1.0f;
	    return 1;
	}
    }
    
    /* if middle mouse was released last frame and click was prev fram
     * then call it a fast stop
     */
    if (mouse->release & button)
    {
	if ((dist < 10) && ((etime < msecs) && (prevClick == button)))
	{
	    prevClick = button;
	    return 1;
	}
    }
    
    if (mouse->click & button)
    {
	prevClick = button;
	prevClickStamp = mouse->clickTime[button];
    }
    
    return 0;
}



PFUIDLLEXPORT pfType *pfiTDFXformerD::classType = NULL;

void
pfiTDFXformerD::init()
{
    if (classType == NULL)
    {
	pfiXformerD::init();
	classType = 
	    new pfType(pfiXformerD::getClassType(), "pfiTDFXformerD");
    }
}

pfiTDFXformerD::pfiTDFXformerD(void)
{
    void *arena		= pfMemory::getArena(this);
    
    setType(classType);
    
    xFastClickTime		   = 300.0f; /* msecs */
    xInputCoordD = new(arena) pfi2DInputCoordD;
    
    xInputD = new(arena) pfiInputD;
    xInputD->setEventStreamProcessor(pfiProcessTDFXformerDMouseEvents, this);
    
    xTrackball = new(arena) pfiInputXformDTrackball;
    xTrackball->setInputCoordDPtr(xInputCoordD);
    xTrackball->setUpdateFunc(pfiUpdate2DIXformDTrackball, NULL);
    xTrackball->xMotionCoordD.startSpeed	= 30.0f;
    
    xDrive = new(arena) pfiInputXformDDrive;
    xDrive->setInputCoordDPtr(xInputCoordD);
    xDrive->setUpdateFunc(pfiUpdate2DIXformDDrive, NULL);
    xDrive->xMotionCoordD.startSpeed	= 0.0f;
    xDrive->xMotionCoordD.minSpeed = -xDrive->xMotionCoordD.maxSpeed;
    xDrive->xMotionCoordD.minAccel = -xDrive->xMotionCoordD.maxAccel;
    
    xFly = new(arena) pfiInputXformDFly;
    xFly->setInputCoordDPtr(xInputCoordD);
    xFly->setUpdateFunc(pfiUpdate2DIXformDFly, NULL);
    xFly->xMotionCoordD.startSpeed	= 0.0f;
    xFly->xMotionCoordD.minSpeed = -xFly->xMotionCoordD.maxSpeed;
    xFly->xMotionCoordD.minAccel = -xFly->xMotionCoordD.maxAccel;
    
    // our traditional trackball had pitch "follow" the mouse
    xFly->setMode(PFIXFLY_MODE_PITCH, PFIXFLY_PITCH_FOLLOW);
    
    xSpheric = new(arena) pfiInputXformDSpheric;
    xSpheric->setInputCoordDPtr(xInputCoordD);
    xSpheric->setUpdateFunc(pfiUpdate2DIXformDSpheric, NULL);
    xSpheric->xMotionCoordD.startSpeed	= 0.0f;
    xSpheric->xMotionCoordD.minSpeed = -xSpheric->xMotionCoordD.maxSpeed;
    xSpheric->xMotionCoordD.minAccel = -xSpheric->xMotionCoordD.maxAccel;

    xDefaultModelIndex = PFITDF_TRACKBALL;
    xModelList->set(PFITDF_TRACKBALL, xTrackball);
    xModelList->set(PFITDF_DRIVE, xDrive);
    xModelList->set(PFITDF_FLY, xFly);
    xModelList->set(PFITDF_SPHERIC, xSpheric);
    reset();
}

pfiTDFXformerD::pfiTDFXformerD(pfiInputXformDTrackball *_tb,
		  pfiInputXformDDrive *_drive,
		  pfiInputXformDFly *_fly,
		  pfiInputXformDSpheric *_spheric)
{
    xTrackball = _tb;
    xDrive = _drive;
    xFly = _fly;
    xSpheric = _spheric;
    xIx = xTrackball;
    xDefaultModelIndex = PFITDF_TRACKBALL;
    xModelList->set(PFITDF_TRACKBALL, xTrackball);
    xModelList->set(PFITDF_DRIVE, xDrive);
    xModelList->set(PFITDF_FLY, xFly);
    xModelList->set(PFITDF_SPHERIC, xSpheric);
    reset();
}

pfiTDFXformerD::~pfiTDFXformerD(void)
{
}

void
pfiTDFXformerD::reset(void)
{
    pfiXformerD::reset();
}

void
pfiTDFXformerD::_setDBLimits(pfBox *_dbLimits)
{
    if (!_dbLimits)
	return;

    pfiXformerD::_setDBLimits(_dbLimits);
}

void	
pfiTDFXformerD::_setBSphere(pfSphere* _sphere)
{
    xDBSphere = *_sphere;
    xDBSize =  2.0f * xDBSphere.radius;
    xDBSizeLg = logf(xDBSize);
    if (!(xUserSetMask & _PFIX_USET_RESET_POS))
    {
	PFCOPY_VEC3(xMotionCoordD.startPos.xyz, xDBSphere.center);
	xMotionCoordD.startPos.xyz[1] -= xDBSize*1.5f;
	xMotionCoordD.startPos.xyz[2] += xDBSphere.radius*0.5f;
	PFSET_VEC3(xMotionCoordD.startPos.hpr, 0.0f, 0.0f, 0.0f);
    }
    if  (!(xUserSetMask & _PFIX_USET_START_MOTION))
    {
	xAccelRate = 2*xDBSizeLg;
    }
}

void
pfiTDFXformerD::setTrackball(pfiInputXformDTrackball *_tb) 
{
    xTrackball = _tb;
    if (xCurModelIndex == PFITDF_TRACKBALL)
	xIx = _tb;
}

void
pfiTDFXformerD::setDrive(pfiInputXformDDrive *_drive) 
{
    xDrive = _drive;
    if (xCurModelIndex == PFITDF_DRIVE)
	xIx = _drive;
}

void
pfiTDFXformerD::setFly(pfiInputXformDFly *_fly) 
{
    xFly = _fly;
    if (xCurModelIndex == PFITDF_FLY)
	xIx = _fly;
}
    

void
pfiTDFXformerD::setSpheric(pfiInputXformDSpheric *_spheric)
{
    xSpheric = _spheric;
    if (xCurModelIndex == PFITDF_SPHERIC)
	xIx = _spheric;
}

void
pfiTDFXformerD::selectModel(int _which)
{
    double n, f;
    if (xCurModelIndex == _which)
	return;

    if (_which > xModelList->getNum())
    {
	pfNotify(PFNFY_WARN, PFNFY_RESOURCE, 
	    "pfiTDFSelectXformerDModel - index %d is more than number of models (%d)", 
	    _which, xModelList->getNum());
	return;
    }
    
    stop();
    
    // The XformerD transfers position changes between motion models.
    // The trackball is special because its matrix edits to the
    // scene DCS matrix must now
    // be applied in reverse to the current position.
    
    if (xCurModelIndex == PFITDF_TRACKBALL)
    {
	pfMatrix4d mat, invmat, posmat;
	xIx->getMat(mat);
	invmat.invertOrtho(mat);
	posmat.makeCoordd(&xMotionCoordD.pos);
	posmat *= invmat;
	posmat.getOrthoCoordd(&xMotionCoordD.pos);
	xMotionCoordD.makeMat(); 
	
	if (xPosDCS)
	    pfDoubleDCSMat(xPosDCS, pfIdentMat4d);
    }
    xCurModelIndex = _which;
    xIx = (pfiInputXformD *) xModelList->get(xCurModelIndex);
    /* init new model with position of old model */
    xIx->setCoord(&(xMotionCoordD.pos));
    /* update current position motion parameters to match motion model */
    xIx->getMotionCoordD(&(xMotionCoordD));
    
    /* update collider with new model */
    xCollide->setCurMotionParams(&xMotionCoordD.pos, &xMotionCoordD.prevPos, 
		xMotionCoordD.speed);
    (pfGetChanNearFar(xInputDChan, (float *)(&n), (float *)(&f)), *(&n) = *(float *)(&n), *(&f) = *(float *)(&f));
    
    if (xIx->isOfType(pfiInputXformDTrackball::getClassType()))
    {
	xCollide->setMode(PFIC_RESPONSE, PFIC_RESPONSE_NONE);
	setCollision(PFUCOLLIDE_GROUND, 0.0f, NULL);
	setCollision(PFUCOLLIDE_OBJECT, 0.0f, NULL);
    }
    else if (xIx->isOfType(pfiInputXformDDrive::getClassType()))
    {
	double dh = ((pfiInputXformDDrive *)xIx)->getDriveHeight();
	xCollide->setMode(PFIC_RESPONSE, PFIC_RESPONSE_STOP);
	setCollision(PFUCOLLIDE_GROUND, dh, xNode);
	setCollision(PFUCOLLIDE_OBJECT, n * 2.0f, xNode);
    }
    else if (xIx->isOfType(pfiInputXformDFly::getClassType()))
    {
	xCollide->setMode(PFIC_RESPONSE, PFIC_RESPONSE_BOUNCE);
	/* Do not terrain follow */
	setCollision(PFUCOLLIDE_GROUND, 0.0f, xNode);
	setCollision(PFUCOLLIDE_OBJECT, n * 2.0f, xNode);
    }    
    else if (xIx->isOfType(pfiInputXformDSpheric::getClassType()))
    {
	xCollide->setMode(PFIC_RESPONSE, PFIC_RESPONSE_STOP);
	    /* Do not terrain follow */
	setCollision(PFUCOLLIDE_GROUND, 0.0f, xNode);
	setCollision(PFUCOLLIDE_OBJECT, n * 2.0f, xNode);
    }
}

void
pfiTDFXformerD::update(void)
{
    if (xInputD)
	xInputD->processEvents();
	
    if (!xIx)
	return;
    
    /* update motion model with info from collide */
    xIx->setMotionCoordD(&(xMotionCoordD));

    if (xIx)
    {
	xIx->update();
	xIx->getMotionCoordD(&(xMotionCoordD));
    }
    
    if (xPosDCS || xPosChan) 
    {
	if (xIx->isOfType(pfiInputXformDTrackball::getClassType()))
	{
	    if (xPosDCS)
	    {
		pfMatrix4d _mat;
		xIx->getMat(_mat);
		pfDoubleDCSMat(xPosDCS, _mat);
	    }
	}
	if (xIx->isOfType(pfiInputXformDTravel::getClassType()))
	{
	    if (xPosChan)
		pfChanViewMatD(xPosChan, xMotionCoordD.mat);
	}
    }
}

int
pfiProcessTDFXformerDMouseEvents(pfiInputD *, pfuEventStream *,  void *data)
{
    pfiTDFXformerD *xf = (pfiTDFXformerD *) data;
    pfuMouse *mouse;
    pfuEventStream *ev;
    pfChannel *chan;
    
    xf->getAutoInputD(&chan, &mouse, &ev);
    
    pfiProcessTDFXformerDMouse(xf, mouse, chan);
    
    return PFI_CB_CONT;
}

void
pfiProcessTDFXformerDMouse(pfiTDFXformerD *xf, pfuMouse *mouse, pfChannel *inputChan)
{
    int focus=1;
    pfiInputXformD *ix;
    
    ix = xf->getCurModel();
    
    /* Only listen to mouse if cursor is in channel and GUI doesn't
     * have focus
     */
    /* XXXX mouse and input handling should be separated from model update !!! */
    if (!(mouse->inWin) || 
	    pfuInGUI(mouse->xpos, mouse->ypos) ||
	    !pfuMouseInChan(mouse, inputChan))
	focus = 0;
    xf->setFocus(focus);
    
    if (!ix)
	return;
	
    ix->setFocus(focus);
    ix->setMode(PFIX_MODE_AUTO, !focus);
    
    if (focus && (mouse->flags & PFUDEV_MOUSE_DOWN_MASK))
    {
	pfiInputCoordD *icoord;
	pfVec2d pos;
	
	pfuCalcNormalizedChanXYd(&pos[0], &pos[1], inputChan, mouse->xpos, mouse->ypos);
	icoord = xf->getInputCoordDPtr();
	icoord->setVec(pos.vec);
    }
    
    /* update the motion model */
    
    if (ix->isOfType(pfiInputXformDTrackball::getClassType()))
	pfiProcessTDFTrackballDMouse(xf, (pfiInputXformDTrackball*)ix, mouse);
    
    else if (ix->isOfType(pfiInputXformDSpheric::getClassType()))
	pfiProcessTDFSphericDMouse(xf, (pfiInputXformDSpheric*)ix, mouse);
    
    else if (ix->isOfType(pfiInputXformDTravel::getClassType()))
	pfiProcessTDFTravelDMouse(xf, (pfiInputXformDTravel*)ix, mouse);
    
    else
    {
	pfNotify(PFNFY_NOTICE, PFNFY_PRINT, "pfiTDFXformerD - Unrecognized type %s - 0x%x", 
	    ix->getTypeName(), ix->getClassType());
	ix->update();
    }
}

void
pfiProcessTDFTrackballDMouse(pfiTDFXformerD *xf, pfiInputXformDTrackball *trackball, pfuMouse *mouse)
{
    int		    buttons, mod;
    int		    accel=PFIXTK_ACCEL_NONE;
    
    buttons = mouse->flags & PFUDEV_MOUSE_DOWN_MASK;
    mod = mouse->flags & PFUDEV_MOD_MASK;
    /* 
     * Button actions:
     *
     *  L  M  R  CODE  INTERPRETATION
     *  -  -  -  ----  ------------------
     *  0  1  0     4  rotate around X and Z axes
     *  0  0  1     2  rotate around Y axis
     *  0  1  1     6  second button accelerates rotate mode chosen by first
     *  1  0  0     1  translate in X and Z planes
     *  1  0  1     5  translate in Y (zoom in and out)
     */
    if (xf->getFocus())
    {
	if (pfiHaveFastMouseClickD(mouse, PFUDEV_MOUSE_MIDDLE_DOWN, xf->getFastClickTime()))
	{
	    trackball->stop();
	}
	else
	{
	    /* hyper acceleration if shift key is down */
	    if (mod & PFUDEV_MOD_SHIFT)
	    {
		accel = PFIXTK_ACCEL_INCREASE;
		trackball->setAccelRate(1.0f);
	    }
	    switch(buttons)
	    {
	    case PFUDEV_MOUSE_MIDDLE_DOWN:
		trackball->setMode(PFIXTK_MODE_MOTION, PFIXTK_MOTION_ROTXZ);
		trackball->setMode(PFIXTK_MODE_AUTO, 0);
		break;
	    case PFUDEV_MOUSE_MIDDLE_DOWN | PFUDEV_MOUSE_RIGHT_DOWN:
		trackball->setMode(PFIXTK_MODE_MOTION, PFIXTK_MOTION_ROTY);
		break;
	    case PFUDEV_MOUSE_LEFT_DOWN:
		trackball->setMode(PFIXTK_MODE_MOTION, PFIXTK_MOTION_TRANSXZ);
		break;
	    case PFUDEV_MOUSE_LEFT_DOWN | PFUDEV_MOUSE_RIGHT_DOWN:
	    case PFUDEV_MOUSE_RIGHT_DOWN:
		trackball->setMode(PFIXTK_MODE_MOTION, PFIXTK_MOTION_TRANSY);
		trackball->setMode(PFIXTK_MODE_ACCEL, accel);
		break;
	    case PFUDEV_MOUSE_LEFT_DOWN | PFUDEV_MOUSE_MIDDLE_DOWN | PFUDEV_MOUSE_RIGHT_DOWN:
		xf->stop();
		trackball->setMode(PFIXTK_MODE_MOTION, PFIXTK_MOTION_STOP);
		break;
	    case 0x0:
	    default:
		trackball->setFocus(0);
		trackball->setMode(PFIX_MODE_AUTO, 1);
		break;
	    }
	}
    }
}


void
pfiProcessTDFSphericDMouse(pfiTDFXformerD *xf, pfiInputXformDSpheric *spheric, pfuMouse *mouse)
{
    int		buttons;

    buttons = mouse->flags & PFUDEV_MOUSE_DOWN_MASK;
    
    /* 
     * Button actions:
     *
     *  L  M  R  CODE  INTERPRETATION
     *  -  -  -  ----  ------------------
     *  1  0  0     ?  in and out
     *  1  1  0     ?  in and out - add rotate
     *	0  1  0	    ?  up and down \(around sphere\), side to side
     *  0  0  1     ?  track in and out
     *  0  1  1     ?  track in and out - add rotate
     */
    if (xf->getFocus())
    {
	if (pfiHaveFastMouseClickD(mouse, PFUDEV_MOUSE_MIDDLE_DOWN, xf->getFastClickTime()))
	{
 	    spheric->setMode(PFIXSPH_MODE_MOTION, PFIXSPH_MOTION_STOP);
	    spheric->stop();
	}
	else
	{
	    spheric->setMode(PFIXSPH_MODE_MOTION_MOD, 0x0);
	    switch(buttons)
	    {
	        case PFUDEV_MOUSE_MIDDLE_DOWN:
	            spheric->xInMotion = TRUE;
		    spheric->xTracking = FALSE;
	            spheric->setMode(PFIXSPH_MODE_MOTION, PFIXSPH_MOTION_AROUND);
		    break;

		case PFUDEV_MOUSE_LEFT_DOWN:
	            spheric->xInMotion = TRUE;
		    spheric->xTracking = FALSE;
		    spheric->setMode(PFIXSPH_MODE_MOTION, PFIXSPH_MOTION_INOUT);
		    break;
		    
	        case PFUDEV_MOUSE_LEFT_DOWN | PFUDEV_MOUSE_MIDDLE_DOWN:
	            spheric->xInMotion = TRUE;
		    spheric->xTracking = FALSE;
		    spheric->setMode(PFIXSPH_MODE_MOTION, PFIXSPH_MOTION_INOUT);
		    spheric->setMode(PFIXSPH_MODE_MOTION_MOD,
				    PFIXSPH_MOTION_MOD_SPIN);
		    break;
		    
		case PFUDEV_MOUSE_RIGHT_DOWN:
	            spheric->xInMotion = TRUE;
		    if (!spheric->xTracking)
			spheric->xTracking = PFIXSPH_PATH_UNKNOWN;
		    spheric->setMode(PFIXSPH_MODE_MOTION,
				    PFIXSPH_MOTION_TRACK_INOUT);
		    break;

		case PFUDEV_MOUSE_RIGHT_DOWN | PFUDEV_MOUSE_MIDDLE_DOWN:
	            spheric->xInMotion = TRUE;
		    if (!spheric->xTracking)
			spheric->xTracking = PFIXSPH_PATH_UNKNOWN;
		    spheric->setMode(PFIXSPH_MODE_MOTION,
				    PFIXSPH_MOTION_TRACK_INOUT);
		    spheric->setMode(PFIXSPH_MODE_MOTION_MOD,
				    PFIXSPH_MOTION_MOD_SPIN);
		    break;
		    
		default:
		case 0x0:
		    // if tracking releasing the button stops
		    spheric->setFocus(0);
		    spheric->setMode(PFIX_MODE_AUTO, 1);
		    break;
		  }
	}
    }
}


void
pfiProcessTDFTravelDMouse(pfiTDFXformerD *xf, pfiInputXformDTravel *tr, pfuMouse *mouse)
{
    int		buttons, mod;
    double	rate = 0.0f;
    double	xfAccelRate;

    buttons = mouse->flags & PFUDEV_MOUSE_DOWN_MASK;
    mod = mouse->flags & PFUDEV_MOD_MASK;
    xfAccelRate = xf->getAccelRate();
    
    /* 
     * Button actions:
     *
     *  L  M  R  CODE  INTERPRETATION
     *  -  -  -  ----  ------------------
     *  1  0  0     4  pan,  accelerate
     *  1  1  0     6  pan,  accelerate
     *  1  0  1     5  stop and pan
     *	0  1  0	    2  drive and steer
     *  0  0  1     1  pan,  decelerate
     *  0  1  1     3  pan,  decelerate
     */
    if (xf->getFocus())
    {
	if (pfiHaveFastMouseClickD(mouse, PFUDEV_MOUSE_MIDDLE_DOWN, xf->getFastClickTime()))
	{
	    tr->stop();
	}
	else
	{
	    tr->setMode(PFIXTR_MODE_MOTION_MOD, 0x0);
	    switch(buttons)
	    {
		case PFUDEV_MOUSE_MIDDLE_DOWN:
		    if (mod & PFUDEV_MOD_CTRL)
		    {
			tr->setMode(PFIXTR_MODE_MOTION_MOD, PFIXTR_MOTION_MOD_VERTICAL);
			if (tr->isOfType(pfiInputXformDDrive::getClassType()))
			{
			    double dh = ((pfiInputXformDDrive *)tr)->getDriveHeight();
			    xf->setCollision(PFUCOLLIDE_GROUND, dh, xf->getNode());
			}
		    }
		    else
		    {
			tr->setMode(PFIXTR_MODE_MOTION, PFIXTR_MOTION_FORWARD);
		    }
		    tr->setMode(PFIXTR_MODE_ACCEL, PFIXTR_ACCEL_NONE);
		    break;
		    
		case PFUDEV_MOUSE_LEFT_DOWN | PFUDEV_MOUSE_MIDDLE_DOWN:
		case PFUDEV_MOUSE_LEFT_DOWN:
		    tr->setMode(PFIXTR_MODE_MOTION, PFIXTR_MOTION_FORWARD);
		    tr->setMode(PFIXTR_MODE_ACCEL, PFIXTR_ACCEL_INCREASE);
		    if (mod & PFUDEV_MOD_SHIFT)
		    {
			rate = PF_MAX2(1.0f, xfAccelRate);
		    }
		    tr->setAccelRate(rate);
		    break;
		    
		case PFUDEV_MOUSE_RIGHT_DOWN | PFUDEV_MOUSE_MIDDLE_DOWN:
		case PFUDEV_MOUSE_RIGHT_DOWN: /* decelerate into reverse */
		    tr->setMode(PFIXTR_MODE_MOTION, PFIXTR_MOTION_FORWARD);
		    tr->setMode(PFIXTR_MODE_ACCEL, PFIXTR_ACCEL_DECREASE);
		    if (mod & PFUDEV_MOD_SHIFT)
		    {
			rate = tr->getAccelRate();
			rate = PF_MAX2(1.0f, xfAccelRate);
		    }
		    tr->setAccelRate(rate);
		    break;
		    
		case PFUDEV_MOUSE_LEFT_DOWN | PFUDEV_MOUSE_RIGHT_DOWN:
		    tr->setMode(PFIXTR_MODE_MOTION, PFIXTR_MOTION_DIRECTION);
		    tr->setMode(PFIXTR_MODE_ACCEL, PFIXTR_ACCEL_NONE);
		    break;
		case PFUDEV_MOUSE_LEFT_DOWN | PFUDEV_MOUSE_MIDDLE_DOWN | PFUDEV_MOUSE_RIGHT_DOWN:
		    xf->stop();
		    tr->setMode(PFIXTR_MODE_MOTION, PFIXTR_MOTION_STOP);
		    break;
		default:
		case 0x0:
		    tr->setFocus(0);
		    tr->setMode(PFIX_MODE_AUTO, 1);
		    tr->setMode(PFIXTR_MODE_ACCEL, PFIXTR_ACCEL_NONE);
		    break;
	    }
	}
    }
}
