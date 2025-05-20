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
 * pfiXformer.h
 *
 * $Revision: 1.29 $
 * $Date: 2002/09/18 02:47:08 $
 *
 */
 
 /*
  * TODO:
  *	o Add processEvents method and event handler - sep from update
  * Drive and Fly:
  *	o start speed is 0
  *	o left accel,  right decel
  *	o mid drive, pan
  *	o fast mid click - stop
  *	o shift - exponential accel
  *	o ctrl - elevator
  */

#ifndef __XFORMER_H__
#define __XFORMER_H__

#include <Performer/pr/pfLinMath.h>
#include <Performer/pr/pfGeoMath.h>
#include <Performer/pr/pfList.h>

#include <Performer/pfui.h>
#include <Performer/pfui/pfiInputXform.h>
#include <Performer/pfui/pfiTrackball.h>
#include <Performer/pfui/pfiDrive.h>
#include <Performer/pfui/pfiFly.h>
#include <Performer/pfui/pfiSpheric.h>
#include <Performer/pfui/pfiCollide.h>

#if PF_CPLUSPLUS_API


class PFUIDLLEXPORT pfiXformer : public pfiInputXform
{
  private:
    static pfType *classType;
  public:
    int				xDefaultModelIndex, xCurModelIndex;
    pfChannel			*xInputChan;
    pfNode			*xNode;
    pfChannel			*xPosChan;
    pfDCS			*xPosDCS;
    pfuMouse			*xMouse;
    
    pfiInputXform		*xIx; // current model
    pfiCollide			*xCollide;
    pfList			*xModelList; /* pfiInputXform list */
    
  public:	/****** constructor/destructors ******/
    pfiXformer(void);
    virtual ~pfiXformer(void);
  public:	/* sets/gets */

    virtual void	setMat(pfMatrix& _mat);
    virtual void	getMat(pfMatrix& _mat);
    virtual void	setModelMat(pfMatrix& _mat);
    virtual void	getModelMat(pfMatrix& _mat);
    virtual void	setCoord(pfCoord *_coord);
    virtual void	setResetCoord(pfCoord *_coord);
    virtual void	getResetCoord(pfCoord *_coord);
    virtual void	setMotionLimits(float _maxSpeed,
				     float _angularVel, float _maxAccel);
    virtual void	setNode(pfNode* _node); 
    virtual pfNode*	getNode(void) { return xNode; }
    virtual void	setBSphere(pfSphere* _sphere);
    virtual void	setAutoInput(pfChannel *_chan, pfuMouse *_mouse, 
			    pfuEventStream *_events);
    virtual void	getAutoInput(pfChannel **_chan, pfuMouse **_mouse, 
			    pfuEventStream **_events);
    virtual void	setAutoPosition(pfChannel *_chan, pfDCS *_dcs);
    virtual void	getAutoPosition(pfChannel **_chan, pfDCS **_dcs) const {
	*_chan = xPosChan;
	*_dcs = xPosDCS;
    }
    virtual void	enableCollision(void);
    virtual void	disableCollision(void);
    virtual int		getCollisionEnable();
    virtual void	setCollision(int _mode, float _val, pfNode *_node);
    virtual int		getCollisionStatus(void);    
  public: 	/****** type *****/
    static void	   init();
    static pfType* getClassType() { return classType; }
    virtual const char*	getTypeName(void) const { return "pfiXformer"; }
  public: 
    virtual void	reset(void);
    virtual void	resetPosition(void);
    virtual void	stop(void);
    virtual void	center(void);
    virtual int		collide(void);
    virtual void	setModel(int _which, pfiInputXform *_icx);
    virtual pfiInputXform*	getModel(int _which);
    virtual pfiInputXform* getCurModel(void) { return xIx; };
    virtual int getCurModelIndex(void) { return xCurModelIndex; };
    virtual void	removeModelIndex(int _which);
    virtual void	removeModel(pfiInputXform *_icx);
    virtual void	selectModel(int _which);
    virtual int		getNumModels(void) { return xModelList->getNum(); }
    virtual void	update(void);
  public: /* internal */
    virtual void	_setDBLimits(pfBox* _dbLimits);
};

#define _PFITDF_USET_DRIVE_HEIGHT 0x00010000

class PFUIDLLEXPORT pfiTDFXformer : public pfiXformer
{
  private: 
    static pfType *classType;
    float			xFastClickTime;
    float			xAccelRate;
    pfiInputXformTrackball	*xTrackball;
    pfiInputXformDrive		*xDrive;
    pfiInputXformFly		*xFly;
    pfiInputXformSpheric	*xSpheric;

  public:	/****** constructor/destructors ******/
    pfiTDFXformer(void);
    virtual ~pfiTDFXformer(void);
    pfiTDFXformer(pfiInputXformTrackball *_tb,
		  pfiInputXformDrive *_drive,
		  pfiInputXformFly *_fly,
		  pfiInputXformSpheric *_spheric);

  public: 	/****** type *****/
    static void	   init();
    static pfType* getClassType() { return classType; }
    virtual const char*	getTypeName(void) const { return "pfiTDFXformer"; }
  public:	/* sets/gets */
    float		getAccelRate(void) { return xAccelRate; }
    virtual void	setFastClickTime(float _click) { xFastClickTime = _click; }
    virtual float	getFastClickTime(void) { return xFastClickTime; }
    virtual void	_setBSphere(pfSphere* _sphere);
    virtual void	_setDBLimits(pfBox *_dbLimits);

	// --- Get/Set the Xform models --- //
    void		setTrackball(pfiInputXformTrackball *_tb);
    pfiInputXformTrackball *getTrackball(void) const { return xTrackball; }
    void		setDrive(pfiInputXformDrive *_drive);
    pfiInputXformDrive *getDrive(void) const { return xDrive; }
    void		setFly(pfiInputXformFly *_fly);
    pfiInputXformFly *getFly(void) const { return xFly; }
    void		setSpheric(pfiInputXformSpheric *_spheric);
    pfiInputXformSpheric *getSpheric(void) const { return xSpheric; }
  public: 	/* general interface */
    virtual void	selectModel(int _which);
    virtual void	update(void);
    virtual void	reset(void);
  private: 
    void		_updateTrackball(void);
    void		_updateTravel(void);
    // void		_updateFly(void);
    void		_updateSpheric(void);
};

#endif /* PF_CPLUSPLUS_API */
#endif /* __XFORMER_H__ */
