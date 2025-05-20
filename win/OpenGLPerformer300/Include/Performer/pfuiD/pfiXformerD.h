/*
 * WARNING! DO NOT EDIT!
 * pfiXformerD.h automatically generated from (../libpfui/pfiXformer.h)
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
 * pfiXformerD.h
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

#ifndef __XFORMER_D_H__
#define __XFORMER_D_H__

#include <Performer/pr/pfLinMath.h>
#include <Performer/pr/pfGeoMath.h>
#include <Performer/pr/pfList.h>

#include <Performer/pfuiD.h>
#include <Performer/pfuiD/pfiInputXformD.h>
#include <Performer/pfuiD/pfiTrackballD.h>
#include <Performer/pfuiD/pfiDriveD.h>
#include <Performer/pfuiD/pfiFlyD.h>
#include <Performer/pfuiD/pfiSphericD.h>
#include <Performer/pfuiD/pfiCollideD.h>

#if PF_CPLUSPLUS_API


class PFUIDLLEXPORT pfiXformerD : public pfiInputXformD
{
  private:
    static pfType *classType;
  public:
    int				xDefaultModelIndex, xCurModelIndex;
    pfChannel			*xInputDChan;
    pfNode			*xNode;
    pfChannel			*xPosChan;
    pfDoubleDCS			*xPosDCS;
    pfuMouse			*xMouse;
    
    pfiInputXformD		*xIx; // current model
    pfiCollideD			*xCollide;
    pfList			*xModelList; /* pfiInputXformD list */
    
  public:	/****** constructor/destructors ******/
    pfiXformerD(void);
    virtual ~pfiXformerD(void);
  public:	/* sets/gets */

    virtual void	setMat(pfMatrix4d& _mat);
    virtual void	getMat(pfMatrix4d& _mat);
    virtual void	setModelMat(pfMatrix4d& _mat);
    virtual void	getModelMat(pfMatrix4d& _mat);
    virtual void	setCoord(pfCoordd *_coord);
    virtual void	setResetCoord(pfCoordd *_coord);
    virtual void	getResetCoord(pfCoordd *_coord);
    virtual void	setMotionLimits(double _maxSpeed,
				     double _angularVel, double _maxAccel);
    virtual void	setNode(pfNode* _node); 
    virtual pfNode*	getNode(void) { return xNode; }
    virtual void	setBSphere(pfSphere* _sphere);
    virtual void	setAutoInputD(pfChannel *_chan, pfuMouse *_mouse, 
			    pfuEventStream *_events);
    virtual void	getAutoInputD(pfChannel **_chan, pfuMouse **_mouse, 
			    pfuEventStream **_events);
    virtual void	setAutoPosition(pfChannel *_chan, pfDoubleDCS *_dcs);
    virtual void	getAutoPosition(pfChannel **_chan, pfDoubleDCS **_dcs) const {
	*_chan = xPosChan;
	*_dcs = xPosDCS;
    }
    virtual void	enableCollision(void);
    virtual void	disableCollision(void);
    virtual int		getCollisionEnable();
    virtual void	setCollision(int _mode, double _val, pfNode *_node);
    virtual int		getCollisionStatus(void);    
  public: 	/****** type *****/
    static void	   init();
    static pfType* getClassType() { return classType; }
    virtual const char*	getTypeName(void) const { return "pfiXformerD"; }
  public: 
    virtual void	reset(void);
    virtual void	resetPosition(void);
    virtual void	stop(void);
    virtual void	center(void);
    virtual int		collide(void);
    virtual void	setModel(int _which, pfiInputXformD *_icx);
    virtual pfiInputXformD*	getModel(int _which);
    virtual pfiInputXformD* getCurModel(void) { return xIx; };
    virtual int getCurModelIndex(void) { return xCurModelIndex; };
    virtual void	removeModelIndex(int _which);
    virtual void	removeModel(pfiInputXformD *_icx);
    virtual void	selectModel(int _which);
    virtual int		getNumModels(void) { return xModelList->getNum(); }
    virtual void	update(void);
  public: /* internal */
    virtual void	_setDBLimits(pfBox* _dbLimits);
};

#define _PFITDF_USET_DRIVE_HEIGHT 0x00010000

class PFUIDLLEXPORT pfiTDFXformerD : public pfiXformerD
{
  private: 
    static pfType *classType;
    double			xFastClickTime;
    double			xAccelRate;
    pfiInputXformDTrackball	*xTrackball;
    pfiInputXformDDrive		*xDrive;
    pfiInputXformDFly		*xFly;
    pfiInputXformDSpheric	*xSpheric;

  public:	/****** constructor/destructors ******/
    pfiTDFXformerD(void);
    virtual ~pfiTDFXformerD(void);
    pfiTDFXformerD(pfiInputXformDTrackball *_tb,
		  pfiInputXformDDrive *_drive,
		  pfiInputXformDFly *_fly,
		  pfiInputXformDSpheric *_spheric);

  public: 	/****** type *****/
    static void	   init();
    static pfType* getClassType() { return classType; }
    virtual const char*	getTypeName(void) const { return "pfiTDFXformerD"; }
  public:	/* sets/gets */
    double		getAccelRate(void) { return xAccelRate; }
    virtual void	setFastClickTime(double _click) { xFastClickTime = _click; }
    virtual double	getFastClickTime(void) { return xFastClickTime; }
    virtual void	_setBSphere(pfSphere* _sphere);
    virtual void	_setDBLimits(pfBox *_dbLimits);

	// --- Get/Set the Xform models --- //
    void		setTrackball(pfiInputXformDTrackball *_tb);
    pfiInputXformDTrackball *getTrackball(void) const { return xTrackball; }
    void		setDrive(pfiInputXformDDrive *_drive);
    pfiInputXformDDrive *getDrive(void) const { return xDrive; }
    void		setFly(pfiInputXformDFly *_fly);
    pfiInputXformDFly *getFly(void) const { return xFly; }
    void		setSpheric(pfiInputXformDSpheric *_spheric);
    pfiInputXformDSpheric *getSpheric(void) const { return xSpheric; }
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
#endif /* __XFORMER_D_H__ */
