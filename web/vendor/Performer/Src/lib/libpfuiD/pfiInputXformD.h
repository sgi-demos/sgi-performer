/*
 * WARNING! DO NOT EDIT!
 * pfiInputXformD.h automatically generated from (../libpfui/pfiInputXform.h)
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
 * pfiInputXformD.h
 *
 * $Revision: 1.31 $
 * $Date: 2002/09/18 02:47:08 $
 *
 */

#ifndef __INPUTXFORM_D_H__
#define __INPUTXFORM_D_H__

#define PFIX_INFPUTXFORM_CLASS	    100
#define PFIX_INFPUTXFORM_TYPE	    100

#include <Performer/pfuiD.h>


extern void _pfiClassInitCheck(void);

#if PF_CPLUSPLUS_API


class PFUIDLLEXPORT pfiInputCoordD : public pfObject
{
  private:
    static pfType *classType;
  public:
    int			focus;
  public:
    pfiInputCoordD(void);
  public:	/****** type *****/
    static void	   init();
    static pfType* getClassType() { return classType; }
    virtual const char*	getTypeName(void) const { return "pfiInputCoordD"; }
  public:	/****** basic interface *****/
    virtual void	setFocus(int _focus) { focus = _focus; }
    virtual int		getFocus(void) const { return (focus != 0); }
    virtual int		inMotion(void) const { return 0; }
    virtual void	reset(void) { focus = 0; }
    virtual void	setVec(double *vec);
    virtual void	setPrev(double *vec);
    virtual void	getVec(double *vec) const;
    virtual void	getPrev(double *vec) const;
    virtual double	getCoord(int index) const;
    virtual double	getPrevCoord(int index) const;
    virtual double	getDelta(int index) const;
    virtual void	updatePrev(void);
    
		/***** parent class copy routines - don't copy what we don't know ****/
    virtual int copy(const pfMemory *_src) {return pfMemory::copy(_src);}
    virtual void copy(const pfObject *_src) {pfObject::copy(_src);}
		/***** our copy routine ****/
    void	copy(pfiInputCoordD *_src) {focus = _src->focus;}
  
};

class PFUIDLLEXPORT pfi2DInputCoordD : public pfiInputCoordD
{
  private:
    static pfType *classType;
  public:
    pfVec2d		pos;
    pfVec2d		prevPos;
  public:	/****** constructor/destructors ******/
    pfi2DInputCoordD(void);
  public:	/****** type *****/
    static void	   init();
    static pfType* getClassType() { return classType; }
    virtual const char*	getTypeName(void) const { return "pfi2DInputCoordD"; }
  public:	/****** sets/gets ******/
    virtual void	reset(void) { 
	PFSET_VEC2(pos, -2.0f, -2.0f);
	PFSET_VEC2(prevPos, -2.0f, -2.0f);
    }
    virtual int		inMotion(void) const {
	if ((prevPos[PF_X] < -1.0f) || (prevPos[PF_Y] < -1.0f))
	    return 0;
	return (!(PFALMOST_EQUAL_VEC2(prevPos, pos, 0.00001f)));
    }
    virtual double	getCoord(int _index) const {
	return	pos[_index];
    }
    virtual double	getPrevCoord(int _index) const {
	return	prevPos[_index];
    }
    virtual double	getDelta(int _index) const {
	if (prevPos[_index] < -1.0f)
	    return 0.0f;
	return (pos[_index] - prevPos[_index]);
    }
    virtual void	getVec(double *vec) const {
	vec[PF_X] = pos[PF_X];
	vec[PF_Y] = pos[PF_Y];
    }
    virtual void	getPrev(double *vec) const {
	vec[PF_X] = prevPos[PF_X];
	vec[PF_Y] = prevPos[PF_Y];
    }
    virtual void	setVec(double *vec) {
	updatePrev();
	PFSET_VEC2(pos, vec[PF_X], vec[PF_Y]);
    }
    virtual void	setPrev(double *vec) {
	PFSET_VEC2(prevPos, vec[PF_X], vec[PF_Y]);
    }
    virtual void	updatePrev(void) {
	PFCOPY_VEC2(prevPos, pos);
    }
};

class PFUIDLLEXPORT pfiMotionCoordD : public pfObject
{
  private:
    static pfType *classType;
  public:
		/* position parameters */
    pfCoordd		startPos, prevPos, pos;
    double		startTime, prevTime, time;
    pfMatrix4d		mat;
		/* motion parameters */
    double		speed, minSpeed, maxSpeed, startSpeed;
    double		angularVel, minAngVel, maxAngVel, startAngVel, angAccel;
    double		accel, minAccel, maxAccel, startAccel;
  public:	/* constructor/destructors */
    pfiMotionCoordD(void);
  public:	/****** type *****/
    static void	   init();
    static pfType* getClassType() { return classType; }
    virtual const char*	getTypeName(void) const { return "pfiMotionCoordD"; }

  public:	/***** parent class copy routines - don't copy what we don't know ****/
    virtual int copy(const pfMemory *_src) {return pfMemory::copy(_src);}
    virtual void copy(const pfObject *_src) {pfObject::copy(_src);}
  public:	/****** basic interface *****/
    void		makeMat(void);
    void		reset(void);
    void		resetPosition(void);
    void		setMat(pfMatrix4d& _mat);
    void		setCoord(pfCoordd *_coord);
		/***** our copy routines ****/
    void		copy(pfiMotionCoordD *_src);
    void		copyPos(pfiMotionCoordD *_src);
    void		copyMotion(pfiMotionCoordD *_src);
  public:
};

class PFUIDLLEXPORT pfiInputD : public pfObject
{
  private:
    static pfType *classType;
  public:
    char		xName[PF_MAXSTRING];
    short		xFocus;
    int			xEventMask;
    pfuEventStream	*xEventStream;
    void		*xEventStreamCollectorData;
    void		*xEventStreamProcessorData;
    void		*xEventHandlerData;

  public:	/****** callbacks and associated CB data *****/
    pfuEventHandlerFuncType	 xHandler; /* called from input process,
					    * gets events not for transform handlers
					    */
    pfiEventStreamHandlerTypeD	xEventStreamCollector;
    pfiEventStreamHandlerTypeD	xEventStreamProcessor;
  public:	/* constructor/destructors */
    pfiInputD(void);
    virtual ~pfiInputD(void);
  public:	/****** sets/gets *****/
    void		setName(const char *_name);
    const char 		*getName(void) {
	return xName;
    }
    void		setEventMask(int _emask) {
	xEventMask = _emask;
    }
    int			getEventMask(void) const { return xEventMask; }
    void		setEventStream(pfuEventStream *_es) {
	xEventStream = _es;
    }
    pfuEventStream	*getEventStream(void) { return xEventStream; }
    void		setEventStreamProcessor(pfiEventStreamHandlerTypeD _func, void *_data) {
		xEventStreamProcessor = _func;
		xEventStreamProcessorData = _data;
    }
    void 		getEventStreamProcessor(pfiEventStreamHandlerTypeD *_func, void **_data) const {
	*_func = xEventStreamProcessor;
	*_data = xEventStreamProcessorData;
    }
    void		setEventStreamCollector(pfiEventStreamHandlerTypeD _func, void *_data) {
		xEventStreamCollector = _func;
		xEventStreamCollectorData = _data;
    }
    void getEventStreamCollector(pfiEventStreamHandlerTypeD *_func, void **_data) const {
	*_func = xEventStreamCollector;
	*_data = xEventStreamCollectorData;
    }
    void		setEventHandler(pfuEventHandlerFuncType _func, void *_data) {
		xHandler = _func;
		xEventHandlerData = _data;
    }
    void 		getEventHandler(pfuEventHandlerFuncType *_func, void **_data) const {
	*_func = xHandler;
	*_data = xEventHandlerData;
    }
    
  public:	/****** type *****/
    static void	   init();
    static pfType* getClassType() { return classType; }
    virtual const char*	getTypeName(void) const { return "pfiInputD"; }
  public:	/****** generic interface *****/
    virtual void	setFocus(int _focus) { xFocus = _focus; }
    virtual int 	getFocus(void) { return (xFocus); }
    virtual void	reset(void);
    virtual void	collectEvents(void) {
	if (xEventStreamCollector)
	    xEventStreamCollector(this, xEventStream, xEventStreamCollectorData);
    }
    virtual void	processEvents(void) {
	if (xEventStreamProcessor)
	    xEventStreamProcessor(this, xEventStream, xEventStreamProcessorData);
    }
};

// masks to record which parameters user has set and need to
// be preserved
#define _PFIX_USET_START_MOTION	    0x001
#define _PFIX_USET_MOTION_LIMITS    0x002
#define _PFIX_USET_DB_LIMITS	    0x004
#define _PFIX_USET_BSPHERE	    0x008
#define _PFIX_USET_DB_SIZE	    0x010
#define _PFIX_USET_RESET_POS	    0x020
#define _PFIX_USET_SPHERIC_LIMITS   0x040

/* limit modes */
#define _PFIX_LIMIT_POS		    0x1
#define _PFIX_LIMIT_SPEED	    0x2
#define _PFIX_LIMIT_ACCEL	    0x4

class PFUIDLLEXPORT pfiInputXformD : public pfObject
{
  private:
    static pfType *classType;
    char		xName[PF_MAXSTRING];
  public:
    short		xFocus;
    pfiMotionCoordD	xMotionCoordD;
    short		xInMotion, xMotionMode, xAccelMode, xAuto;
    int			xUpdateMode;
    int			xLimitPosMask, xLimitSpeedMask, xLimitAccelMask;
    double		xDBSize; /* diameter of xDBSphere */
    double		xDBSizeLg; /* log of DBSize */
    pfBox		xDBLimits; /* tight bounding box on database */
    pfSphere		xDBSphere; /* bounding sphere for xDBLimits */
    int			xUserSetMask; /* mask of user set props, tells us not to auto set */
    pfiInputD		*xInputD;
    pfiInputCoordD	*xInputCoordD;
    void		*xMotionCBData;
    pfiInputXformDFuncType	xStartMotionCB, xStopMotionCB;
    void		*xUpdateCBData;
    pfiInputXformDUpdateFuncType xUpdateCB;
  public:	/****** constructor/destructors ******/
    pfiInputXformD(void);
  public:	/****** type ******/
    static void	   init();
    static pfType* getClassType() { return classType; }
    virtual const char*	getTypeName(void) const { return "pfiInputXformD"; }
  public:	/****** generic interface ******/
    virtual void	reset(void);
    virtual void	resetPosition(void);
    virtual void	stop(void);
    virtual int		isInMotion(void) const { return xInMotion; }
    virtual void	limitPos(void);
    virtual void	limitSpeed(void);
    virtual void	update(void);
    void		setName(const char *_name);
    const char 		*getName(void) {
	return xName;
    }
    virtual void	setFocus(int _focus) { xFocus = _focus; }
    virtual int 	getFocus(void) { return (xFocus); }
    virtual void	setMode(int _mode, int _val);
    virtual int 	getMode(int _mode) const;
    virtual void		setMat(pfMatrix4d& _mat);
    virtual void		getMat(pfMatrix4d& _mat) {
	PFCOPY_MAT(_mat,  xMotionCoordD.mat);
    }
    virtual void	setInputCoordDPtr(pfiInputCoordD *_ic) { xInputCoordD = _ic; }
    virtual pfiInputCoordD* getInputCoordDPtr(void) { return xInputCoordD; }
    virtual void	setInputD(pfiInputD *_in) { xInputD = _in; }
    virtual pfiInputD*	getInputD(void) { return xInputD; }
    virtual void	setMotionCoordD(pfiMotionCoordD *_coord);
    virtual void	getMotionCoordD(pfiMotionCoordD *_coord);
    virtual void	setCoord(pfCoordd *_coord);
    virtual void	getCoord(pfCoordd *_coord) {
	*_coord = xMotionCoordD.pos;
    }
    virtual void	setResetCoord(pfCoordd *_coord) {
	xMotionCoordD.startPos = *_coord;
	xUserSetMask |= _PFIX_USET_RESET_POS;
    }
    virtual void	getResetCoord(pfCoordd *_coord) {
	*_coord = xMotionCoordD.startPos;
    }
    virtual void	setStartSpeed(double _speed) {
	xMotionCoordD.startSpeed = _speed;
    }
    virtual double	getStartSpeed(void) const {
	return xMotionCoordD.startSpeed; 
    }
    virtual void	setUpdateFunc(pfiInputXformDUpdateFuncType _updateCB, 
				void *_updateData) {
	xUpdateCB =_updateCB;
	xUpdateCBData = _updateData;	   
    }
    virtual void	getUpdateFunc(pfiInputXformDUpdateFuncType *_updateCB, 
				void **_updateData) {
	*_updateCB = xUpdateCB;
	*_updateData = xUpdateCBData;	   
    }
    // XXX Motion callbacks and isInMotion aren't really the same thing
    virtual void	setMotionFuncs(pfiInputXformDFuncType _startCB, 
				       pfiInputXformDFuncType _stopCB, 
				       void *_motionData) {
	xStartMotionCB = _startCB;
	xStopMotionCB =_stopCB;
	xMotionCBData = _motionData;	   
   }
   virtual void		getMotionFuncs(pfiInputXformDFuncType *_startCB, 
				       pfiInputXformDFuncType *_stopCB, 
				       void **_motionData) {
	*_startCB = xStartMotionCB;
	*_stopCB = xStopMotionCB;
	*_motionData = xMotionCBData;
    }
    virtual void	setStartMotion(double _startSpeed, double _startAccel) {
	xUserSetMask |= _PFIX_USET_START_MOTION;
	xMotionCoordD.startSpeed = _startSpeed;
	xMotionCoordD.accel = xMotionCoordD.startAccel = _startAccel;
    }
    virtual void	getStartMotion(double *_startSpeed,
				     double *_startAccel) const {
	*_startSpeed = xMotionCoordD.startSpeed;
	*_startAccel = xMotionCoordD.startAccel;
    }
    virtual void	setMotionLimits(double _maxSpeed,
				     double _angularVel, double _maxAccel);
    virtual void	getMotionLimits(double *_maxSpeed,
				     double *_angularVel, double *_maxAccel) const;
    virtual void	setDBLimits(pfBox* _dbLimits) {
	_setDBLimits(_dbLimits);
	xUserSetMask |= _PFIX_USET_DB_LIMITS;
    }
    virtual void	setBSphere(pfSphere* _sphere) {
	_setBSphere(_sphere);
	xUserSetMask |= _PFIX_USET_BSPHERE;
    }
    virtual void	_setDBLimits(pfBox* _dbLimits);
    virtual void	getDBLimits(pfBox* _dbLimits) const { *_dbLimits = xDBLimits; }
    virtual void	_setBSphere(pfSphere* _sphere);
    virtual void	getBSphere(pfSphere* _sphere) const { *_sphere = xDBSphere; }
    virtual void	setCenter(double _x, double _y, double _z); /* setCenter updates BSphere */
    virtual void	getCenter(double *_x, double *_y, double *_z) const {
	*_x = xDBSphere.center[0];
	*_y = xDBSphere.center[1];
	*_z = xDBSphere.center[2];
    }
    virtual void	setDBSize(double _size); /* updates BSphere */
    virtual double	getDBSize(void) const { return xDBSize; }
};

class PFUIDLLEXPORT pfiInputXformDTravel : public pfiInputXformD
{
  private:
    static pfType *classType;
  public:
    int			xMotionMod;
    double		xAccelRate; /* extra acceleration/deceleration multiplier */
  public:	/****** constructor/destructors ******/
    pfiInputXformDTravel(void);
  public:	/****** type *****/
    static void	   init();
    static pfType* getClassType() { return classType; }
    virtual const char*	getTypeName(void) const { return "pfiInputXformDTravel"; }
  public:	/***** sets/gets *****/
    void		setAccelRate(double _m) { xAccelRate = _m; }
    double		getAccelRate(void) { return xAccelRate; }
  public: 	/**** generic interface ****/
    virtual void	setMode(int _mode, int _val);
    virtual int		getMode(int _mode) const;
    virtual void	reset(void);
    virtual void	stop(void);
};

#endif /* PF_CPLUSPLUS_API */

#endif /* __INPUTXFORM_D_H__ */
