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
 * pfiXformer.C
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
#include <Performer/pf/pfDCS.h>
#include <Performer/pf/pfNode.h>
#include <Performer/pr/pfMemory.h>


#define _PFI_XFORMER_C_
#include <Performer/pfui.h>
#include <Performer/pfui/pfiXformer.h>
#include <Performer/pr/pfGeoSet.h>


PFUIDLLEXPORT pfType *pfiXformer::classType = NULL;

void
pfiXformer::init()
{
    if (classType == NULL)
    {
	pfMemory::init();
	classType = 
	    new pfType(pfiXformer::getClassType(), "pfiXformer");
    }
}


pfiXformer::pfiXformer(void)
{
    void *arena		= pfMemory::getArena(this);
    
    setType(classType);
    xDefaultModelIndex = 0;
    xInputChan             = NULL;
    xPosChan		   = NULL;
    xNode		   = NULL;
    xPosDCS		   = NULL;
    xMouse                 = NULL;
    xIx			   = NULL;
    xModelList = new(arena) pfList;
    xCollide = pfiNewCollide(arena);
}

pfiXformer::~pfiXformer(void)
{
}

void
pfiXformer::reset(void)
{
    int i, numModels;
    pfiInputXform	*icx;
    pfiInputXform::reset();
    xCurModelIndex            = xDefaultModelIndex;
    if ((xDefaultModelIndex > -1) && (xModelList->getNum() > xDefaultModelIndex))
	xIx = (pfiInputXform*) xModelList->get(xDefaultModelIndex);
    else
	xIx = NULL;
    numModels = xModelList->getNum();
    for (i=0; i < numModels; i++)
    {
	icx = (pfiInputXform*) xModelList->get(i);
	if (icx)
	    icx->reset();
    }

    xIx->getMotionCoord(&(xMotionCoord));
    xCollide->reset();
}

void
pfiXformer::resetPosition(void)
{
    xIx->resetPosition();
    xIx->getMotionCoord(&(xMotionCoord));
    xCollide->setCurMotionParams(&xMotionCoord.pos, &xMotionCoord.prevPos, 
		xMotionCoord.speed);
    if (xIx->isOfType(pfiInputXformDrive::getClassType()))
    {
	float dh = ((pfiInputXformDrive *)xIx)->getDriveHeight();
	setCollision(PFUCOLLIDE_GROUND, dh, getNode());
    }
    
    if (xPosChan)
     	pfChanViewMat(xPosChan, xMotionCoord.mat);

    if (xPosDCS)
	pfDCSMat(xPosDCS, pfIdentMat);
}

void
pfiXformer::center(void)
{
    pfCoord center;

    PFCOPY_VEC3(center.xyz, xDBSphere.center);
    PFSET_VEC3(center.hpr, 0.0f, 0.0f, 0.0f);
    xIx->setCoord(&center);
    xIx->getMotionCoord(&(xMotionCoord));
    xCollide->setCurMotionParams(&xMotionCoord.pos, &xMotionCoord.prevPos, 
		xMotionCoord.speed);
    if (xIx->isOfType(pfiInputXformDrive::getClassType()))
    {
	float dh = ((pfiInputXformDrive *)xIx)->getDriveHeight();
	setCollision(PFUCOLLIDE_GROUND, dh, getNode());
    }
		
    if (xPosChan)
	pfChanViewMat(xPosChan, xMotionCoord.mat);
}

void
pfiXformer::setModel(int _which, pfiInputXform *model)
{
    xModelList->set(_which, model) ;
}

void
pfiXformer::selectModel(int _which)
{
    if (_which > xModelList->getNum())
    {
	pfNotify(PFNFY_WARN, PFNFY_RESOURCE, 
	    "pfiSelectXformerModel - index %d is more than number of models (%d)", 
	    _which, xModelList->getNum());
	return;
    }
    stop();
    xIx = (pfiInputXform *) xModelList->get(_which) ;
}

pfiInputXform *
pfiXformer::getModel(int _which)
{
    if (_which > xModelList->getNum())
    {
	pfNotify(PFNFY_WARN, PFNFY_RESOURCE, 
	    "pfiGetXformerModel - index %d is more than number of models (%d)", 
	    _which, xModelList->getNum());
	return NULL;
    }
    return (pfiInputXform*) xModelList->get(_which) ;
}

void	
pfiXformer::removeModelIndex(int _which)
{
    if (_which > xModelList->getNum())
    {
	pfNotify(PFNFY_WARN, PFNFY_RESOURCE, 
	    "pfiRemoveXformerModel - index %d is more than number of models (%d)", 
	    _which, xModelList->getNum());
	return;
    }
    xModelList->set(_which, NULL);
}

void	
pfiXformer::removeModel(pfiInputXform *_icx)
{
    int i, numModels, found = 0;
    pfiInputXform	*icx;
    numModels = xModelList->getNum();
    for (i=0; i < numModels; i++)
    {
	icx = (pfiInputXform*) xModelList->get(i);
	if (icx == _icx)
	{
	    xModelList->set(i, NULL);
	    found = 1;
	    break;
	}
    }
    if (!found)
	pfNotify(PFNFY_WARN, PFNFY_RESOURCE, 
	    "pfiRemoveXformerModel - couldn't find model 0x%p to remove.", _icx);
}

void
pfiXformer::stop(void)
{
    pfiInputXform::stop();
    if (xIx)
    {
	xIx->stop();
	xIx->getMotionCoord(&(xMotionCoord));
    }
    xCollide->setCurMotionParams(&xMotionCoord.pos, &xMotionCoord.prevPos, 
		xMotionCoord.speed);
}

void
pfiXformer::update(void)
{    

    // process input events
    if (xInput)
	xInput->processEvents();
	
    if (xIx)
    {
	/* update motion model with info from collide */
	xIx->setMotionCoord(&(xMotionCoord));
	xIx->update();
    }
    
    xIx->getMotionCoord(&(xMotionCoord));
}

void
pfiXformer::setAutoInput(pfChannel *_chan, pfuMouse *_mouse, 
    pfuEventStream *_events)
{
    xMouse  = _mouse;
    xInputChan   = _chan;
    if (xInput)
	xInput->setEventStream(_events);
    pfuCollisionChan(xInputChan);
}

void
pfiXformer::getAutoInput(pfChannel **_chan, pfuMouse **_mouse, 
    pfuEventStream **_events)
{
    *_mouse = xMouse;
    *_chan = xInputChan;
    if (xInput)
	*_events = xInput->getEventStream();
    else
	*_events = NULL;
}

void
pfiXformer::setMat(pfMatrix& _mat)
{
    int i, numModels;
    pfiInputXform	*icx;
    
    numModels = xModelList->getNum();
    for (i=0; i < numModels; i++)
    {
	icx = (pfiInputXform*) xModelList->get(i);
	if (icx)
	    icx->setMat(_mat);
    }
    if (xIx)
	xIx->getMotionCoord(&(xMotionCoord));
    xCollide->setCurMotionParams(&xMotionCoord.pos, &xMotionCoord.prevPos, 
		xMotionCoord.speed);
}

void
pfiXformer::getMat(pfMatrix& _mat)
{
    PFCOPY_MAT(_mat,  xMotionCoord.mat);
}



void
pfiXformer::setModelMat(pfMatrix& _mat)
{	    
    if (xIx)
    {
	xIx->setMat(_mat);
	xIx->getMotionCoord(&(xMotionCoord));
    }
    xCollide->setCurMotionParams(&xMotionCoord.pos, &xMotionCoord.prevPos, 
		xMotionCoord.speed);
}

void
pfiXformer::getModelMat(pfMatrix& _mat)
{
    if (xIx)
	xIx->getMat(_mat);
}


void
pfiXformer::setResetCoord(pfCoord *_coord) 
{
    int i, numModels;
    pfiInputXform	*icx;
    
    xMotionCoord.startPos = *_coord;
    numModels = xModelList->getNum();
    for (i=0; i < numModels; i++)
    {
	icx = (pfiInputXform*) xModelList->get(i);
	if (icx)
	    icx->setResetCoord(_coord);
    }
	
    xUserSetMask |= _PFIX_USET_RESET_POS;
}

void
pfiXformer::getResetCoord(pfCoord *_coord) 
{
    *_coord = xMotionCoord.startPos;
}
    
void
pfiXformer::setCoord(pfCoord *_coord)
{
    int i, numModels;
    pfiInputXform	*icx;
    
    if (_coord == NULL)
	return;
  
    numModels = xModelList->getNum();
    for (i=0; i < numModels; i++)
    {
	icx = (pfiInputXform*) xModelList->get(i);
	if (icx)
	    icx->setCoord(_coord);
    }
    if (xIx)
	xIx->getMotionCoord(&(xMotionCoord));
	
    xCollide->setCurMotionParams(&xMotionCoord.pos, &xMotionCoord.prevPos, 
		xMotionCoord.speed);
}

void
pfiXformer::setMotionLimits(float _maxSpeed, float _angularVel, float _accel)
{
    int i, numModels;
    pfiInputXform	*icx;
    
    pfiInputXform::setMotionLimits(_maxSpeed, _angularVel, _accel);
    numModels = xModelList->getNum();
    for (i=0; i < numModels; i++)
    {
	icx = (pfiInputXform*) xModelList->get(i);
	if (icx)
	    icx->setMotionLimits(_maxSpeed, _angularVel, _accel);
    }
}

void
pfiXformer::setAutoPosition(pfChannel *_chan, pfDCS *_dcs)
{ 
    if (_dcs)
	_dcs->ref(); 
    if (xPosDCS)
	xPosDCS->unref();
    xPosChan = _chan; 
    xPosDCS = _dcs; 
}

void
pfiXformer::setBSphere(pfSphere *_sphere)
{
    int i, numModels;
    pfiInputXform	*icx;
    
    xDBSphere = *_sphere;
    xUserSetMask |= _PFIX_USET_BSPHERE;
    
    numModels = xModelList->getNum();
    for (i=0; i < numModels; i++)
    {
	icx = (pfiInputXform*) xModelList->get(i);
	if (icx)
	    icx->setBSphere(_sphere);
    }
}

void
pfiXformer::setNode(pfNode *_node)
{ 
    pfBox bbox;
    pfSphere sph;
    int bound;
    float bmax=0.0f;
    
    if (_node)
	_node->ref(); 
    if (xNode)
	xNode->unref();
    xNode = _node; 
    
    // See if the user has set a bounding sphere on the node
    // handle has fixed sphere on the Xformer
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
	    pfSetVec3(bbox.min, sph.center[0] - sph.radius, 
		sph.center[1] - sph.radius, 
		sph.center[2] - sph.radius);
	    pfSetVec3(bbox.max,  sph.center[0] + sph.radius, 
		sph.center[1] + sph.radius, 
		sph.center[2] + sph.radius);
	}
	_setDBLimits(&bbox);
    }
}

void
pfiXformer::_setDBLimits(pfBox *_dbLimits)
{
    int i, numModels;
    pfiInputXform	*icx;
    
    if (!_dbLimits)
	return;

    pfiInputXform::_setDBLimits(_dbLimits); // udpates the BSphere
    numModels = xModelList->getNum();
    for (i=0; i < numModels; i++)
    {
	icx = (pfiInputXform*) xModelList->get(i);
	if (icx)
	    icx->_setDBLimits(_dbLimits);
    }
}

void
pfiXformer::setCollision(int _mode, float _val, pfNode *_node)
{
    int cmode;
	
    cmode = pfiGetCollideMode(xCollide, PFIC_TARGET);
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
pfiXformer::collide(void)
{
    int ret;
    
    pfuCollisionChan(xInputChan);
    
    /* update the collider with new position and motion from motion model */
    xCollide->setCurMotionParams(&xMotionCoord.pos, &xMotionCoord.prevPos, 
		xMotionCoord.speed);
    ret = xCollide->update();
    
    /* If collisions are enabled,
     * get prev position back from collider (may be running in 
     * a separate asynch process but partial updates are probably ok).
     */
    if (xCollide->getMode(PFIC_TARGET))
    {
	xCollide->getCurMotionParams(&xMotionCoord.pos, &xMotionCoord.prevPos, 
		&xMotionCoord.speed);
	xMotionCoord.makeMat();
    }
    return ret;
}

void
pfiXformer::enableCollision(void)
{
    if (xCollide)
	xCollide->enable();
}

void
pfiXformer::disableCollision(void)
{
    if (xCollide)
	xCollide->disable();
}

int
pfiXformer::getCollisionEnable(void)
{
    if (xCollide)
	return xCollide->getEnable();
    else
	return 0;
}

int
pfiXformer::getCollisionStatus(void)
{
    if (xCollide)
	return xCollide->getStatus();
    else
	return 0;
}

/*
 * pfiHaveFastMouseClick() detects fast mouse clicks 
 */
int
pfiHaveFastMouseClick(pfuMouse *mouse, int button, float msecs)
{
    int dist;
    float etime=0.0f;
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



PFUIDLLEXPORT pfType *pfiTDFXformer::classType = NULL;

void
pfiTDFXformer::init()
{
    if (classType == NULL)
    {
	pfiXformer::init();
	classType = 
	    new pfType(pfiXformer::getClassType(), "pfiTDFXformer");
    }
}

pfiTDFXformer::pfiTDFXformer(void)
{
    void *arena		= pfMemory::getArena(this);
    
    setType(classType);
    
    xFastClickTime		   = 300.0f; /* msecs */
    xInputCoord = new(arena) pfi2DInputCoord;
    
    xInput = new(arena) pfiInput;
    xInput->setEventStreamProcessor(pfiProcessTDFXformerMouseEvents, this);
    
    xTrackball = new(arena) pfiInputXformTrackball;
    xTrackball->setInputCoordPtr(xInputCoord);
    xTrackball->setUpdateFunc(pfiUpdate2DIXformTrackball, NULL);
    xTrackball->xMotionCoord.startSpeed	= 30.0f;
    
    xDrive = new(arena) pfiInputXformDrive;
    xDrive->setInputCoordPtr(xInputCoord);
    xDrive->setUpdateFunc(pfiUpdate2DIXformDrive, NULL);
    xDrive->xMotionCoord.startSpeed	= 0.0f;
    xDrive->xMotionCoord.minSpeed = -xDrive->xMotionCoord.maxSpeed;
    xDrive->xMotionCoord.minAccel = -xDrive->xMotionCoord.maxAccel;
    
    xFly = new(arena) pfiInputXformFly;
    xFly->setInputCoordPtr(xInputCoord);
    xFly->setUpdateFunc(pfiUpdate2DIXformFly, NULL);
    xFly->xMotionCoord.startSpeed	= 0.0f;
    xFly->xMotionCoord.minSpeed = -xFly->xMotionCoord.maxSpeed;
    xFly->xMotionCoord.minAccel = -xFly->xMotionCoord.maxAccel;
    
    // our traditional trackball had pitch "follow" the mouse
    xFly->setMode(PFIXFLY_MODE_PITCH, PFIXFLY_PITCH_FOLLOW);
    
    xSpheric = new(arena) pfiInputXformSpheric;
    xSpheric->setInputCoordPtr(xInputCoord);
    xSpheric->setUpdateFunc(pfiUpdate2DIXformSpheric, NULL);
    xSpheric->xMotionCoord.startSpeed	= 0.0f;
    xSpheric->xMotionCoord.minSpeed = -xSpheric->xMotionCoord.maxSpeed;
    xSpheric->xMotionCoord.minAccel = -xSpheric->xMotionCoord.maxAccel;

    xDefaultModelIndex = PFITDF_TRACKBALL;
    xModelList->set(PFITDF_TRACKBALL, xTrackball);
    xModelList->set(PFITDF_DRIVE, xDrive);
    xModelList->set(PFITDF_FLY, xFly);
    xModelList->set(PFITDF_SPHERIC, xSpheric);
    reset();
}

pfiTDFXformer::pfiTDFXformer(pfiInputXformTrackball *_tb,
		  pfiInputXformDrive *_drive,
		  pfiInputXformFly *_fly,
		  pfiInputXformSpheric *_spheric)
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

pfiTDFXformer::~pfiTDFXformer(void)
{
}

void
pfiTDFXformer::reset(void)
{
    pfiXformer::reset();
}

void
pfiTDFXformer::_setDBLimits(pfBox *_dbLimits)
{
    if (!_dbLimits)
	return;

    pfiXformer::_setDBLimits(_dbLimits);
}

void	
pfiTDFXformer::_setBSphere(pfSphere* _sphere)
{
    xDBSphere = *_sphere;
    xDBSize =  2.0f * xDBSphere.radius;
    xDBSizeLg = logf(xDBSize);
    if (!(xUserSetMask & _PFIX_USET_RESET_POS))
    {
	PFCOPY_VEC3(xMotionCoord.startPos.xyz, xDBSphere.center);
	xMotionCoord.startPos.xyz[1] -= xDBSize*1.5f;
	xMotionCoord.startPos.xyz[2] += xDBSphere.radius*0.5f;
	PFSET_VEC3(xMotionCoord.startPos.hpr, 0.0f, 0.0f, 0.0f);
    }
    if  (!(xUserSetMask & _PFIX_USET_START_MOTION))
    {
	xAccelRate = 2*xDBSizeLg;
    }
}

void
pfiTDFXformer::setTrackball(pfiInputXformTrackball *_tb) 
{
    xTrackball = _tb;
    if (xCurModelIndex == PFITDF_TRACKBALL)
	xIx = _tb;
}

void
pfiTDFXformer::setDrive(pfiInputXformDrive *_drive) 
{
    xDrive = _drive;
    if (xCurModelIndex == PFITDF_DRIVE)
	xIx = _drive;
}

void
pfiTDFXformer::setFly(pfiInputXformFly *_fly) 
{
    xFly = _fly;
    if (xCurModelIndex == PFITDF_FLY)
	xIx = _fly;
}
    

void
pfiTDFXformer::setSpheric(pfiInputXformSpheric *_spheric)
{
    xSpheric = _spheric;
    if (xCurModelIndex == PFITDF_SPHERIC)
	xIx = _spheric;
}

void
pfiTDFXformer::selectModel(int _which)
{
    float n, f;
    if (xCurModelIndex == _which)
	return;

    if (_which > xModelList->getNum())
    {
	pfNotify(PFNFY_WARN, PFNFY_RESOURCE, 
	    "pfiTDFSelectXformerModel - index %d is more than number of models (%d)", 
	    _which, xModelList->getNum());
	return;
    }
    
    stop();
    
    // The Xformer transfers position changes between motion models.
    // The trackball is special because its matrix edits to the
    // scene DCS matrix must now
    // be applied in reverse to the current position.
    
    if (xCurModelIndex == PFITDF_TRACKBALL)
    {
	pfMatrix mat, invmat, posmat;
	xIx->getMat(mat);
	invmat.invertOrtho(mat);
	posmat.makeCoord(&xMotionCoord.pos);
	posmat *= invmat;
	posmat.getOrthoCoord(&xMotionCoord.pos);
	xMotionCoord.makeMat(); 
	
	if (xPosDCS)
	    pfDCSMat(xPosDCS, pfIdentMat);
    }
    xCurModelIndex = _which;
    xIx = (pfiInputXform *) xModelList->get(xCurModelIndex);
    /* init new model with position of old model */
    xIx->setCoord(&(xMotionCoord.pos));
    /* update current position motion parameters to match motion model */
    xIx->getMotionCoord(&(xMotionCoord));
    
    /* update collider with new model */
    xCollide->setCurMotionParams(&xMotionCoord.pos, &xMotionCoord.prevPos, 
		xMotionCoord.speed);
    pfGetChanNearFar(xInputChan, &n, &f);
    
    if (xIx->isOfType(pfiInputXformTrackball::getClassType()))
    {
	xCollide->setMode(PFIC_RESPONSE, PFIC_RESPONSE_NONE);
	setCollision(PFUCOLLIDE_GROUND, 0.0f, NULL);
	setCollision(PFUCOLLIDE_OBJECT, 0.0f, NULL);
    }
    else if (xIx->isOfType(pfiInputXformDrive::getClassType()))
    {
	float dh = ((pfiInputXformDrive *)xIx)->getDriveHeight();
	xCollide->setMode(PFIC_RESPONSE, PFIC_RESPONSE_STOP);
	setCollision(PFUCOLLIDE_GROUND, dh, xNode);
	setCollision(PFUCOLLIDE_OBJECT, n * 2.0f, xNode);
    }
    else if (xIx->isOfType(pfiInputXformFly::getClassType()))
    {
	xCollide->setMode(PFIC_RESPONSE, PFIC_RESPONSE_BOUNCE);
	/* Do not terrain follow */
	setCollision(PFUCOLLIDE_GROUND, 0.0f, xNode);
	setCollision(PFUCOLLIDE_OBJECT, n * 2.0f, xNode);
    }    
    else if (xIx->isOfType(pfiInputXformSpheric::getClassType()))
    {
	xCollide->setMode(PFIC_RESPONSE, PFIC_RESPONSE_STOP);
	    /* Do not terrain follow */
	setCollision(PFUCOLLIDE_GROUND, 0.0f, xNode);
	setCollision(PFUCOLLIDE_OBJECT, n * 2.0f, xNode);
    }
}

void
pfiTDFXformer::update(void)
{
    if (xInput)
	xInput->processEvents();
	
    if (!xIx)
	return;
    
    /* update motion model with info from collide */
    xIx->setMotionCoord(&(xMotionCoord));

    if (xIx)
    {
	xIx->update();
	xIx->getMotionCoord(&(xMotionCoord));
    }
    
    if (xPosDCS || xPosChan) 
    {
	if (xIx->isOfType(pfiInputXformTrackball::getClassType()))
	{
	    if (xPosDCS)
	    {
		pfMatrix _mat;
		xIx->getMat(_mat);
		pfDCSMat(xPosDCS, _mat);
	    }
	}
	if (xIx->isOfType(pfiInputXformTravel::getClassType()))
	{
	    if (xPosChan)
		pfChanViewMat(xPosChan, xMotionCoord.mat);
	}
    }
}

int
pfiProcessTDFXformerMouseEvents(pfiInput *, pfuEventStream *,  void *data)
{
    pfiTDFXformer *xf = (pfiTDFXformer *) data;
    pfuMouse *mouse;
    pfuEventStream *ev;
    pfChannel *chan;
    
    xf->getAutoInput(&chan, &mouse, &ev);
    
    pfiProcessTDFXformerMouse(xf, mouse, chan);
    
    return PFI_CB_CONT;
}

void
pfiProcessTDFXformerMouse(pfiTDFXformer *xf, pfuMouse *mouse, pfChannel *inputChan)
{
    int focus=1;
    pfiInputXform *ix;
    
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
	pfiInputCoord *icoord;
	pfVec2 pos;
	
	pfuCalcNormalizedChanXY(&pos[0], &pos[1], inputChan, mouse->xpos, mouse->ypos);
	icoord = xf->getInputCoordPtr();
	icoord->setVec(pos.vec);
    }
    
    /* update the motion model */
    
    if (ix->isOfType(pfiInputXformTrackball::getClassType()))
	pfiProcessTDFTrackballMouse(xf, (pfiInputXformTrackball*)ix, mouse);
    
    else if (ix->isOfType(pfiInputXformSpheric::getClassType()))
	pfiProcessTDFSphericMouse(xf, (pfiInputXformSpheric*)ix, mouse);
    
    else if (ix->isOfType(pfiInputXformTravel::getClassType()))
	pfiProcessTDFTravelMouse(xf, (pfiInputXformTravel*)ix, mouse);
    
    else
    {
	pfNotify(PFNFY_NOTICE, PFNFY_PRINT, "pfiTDFXformer - Unrecognized type %s - 0x%x", 
	    ix->getTypeName(), ix->getClassType());
	ix->update();
    }
}

void
pfiProcessTDFTrackballMouse(pfiTDFXformer *xf, pfiInputXformTrackball *trackball, pfuMouse *mouse)
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
	if (pfiHaveFastMouseClick(mouse, PFUDEV_MOUSE_MIDDLE_DOWN, xf->getFastClickTime()))
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
pfiProcessTDFSphericMouse(pfiTDFXformer *xf, pfiInputXformSpheric *spheric, pfuMouse *mouse)
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
	if (pfiHaveFastMouseClick(mouse, PFUDEV_MOUSE_MIDDLE_DOWN, xf->getFastClickTime()))
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
pfiProcessTDFTravelMouse(pfiTDFXformer *xf, pfiInputXformTravel *tr, pfuMouse *mouse)
{
    int		buttons, mod;
    float	rate = 0.0f;
    float	xfAccelRate;

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
	if (pfiHaveFastMouseClick(mouse, PFUDEV_MOUSE_MIDDLE_DOWN, xf->getFastClickTime()))
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
			if (tr->isOfType(pfiInputXformDrive::getClassType()))
			{
			    float dh = ((pfiInputXformDrive *)tr)->getDriveHeight();
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
