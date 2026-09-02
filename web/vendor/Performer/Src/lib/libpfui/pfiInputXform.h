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
 * pfiInputXform.h
 *
 * $Revision: 1.31 $
 * $Date: 2002/09/18 02:47:08 $
 *
 */

#ifndef __INPUTXFORM_H__
#define __INPUTXFORM_H__

#define PFIX_INFPUTXFORM_CLASS	    100
#define PFIX_INFPUTXFORM_TYPE	    100

#include <Performer/pfui.h>


extern void _pfiClassInitCheck(void);

#if PF_CPLUSPLUS_API


class PFUIDLLEXPORT pfiInputCoord : public pfObject
{
  private:
    static pfType *classType;
  public:
    int			focus;
  public:
    pfiInputCoord(void);
  public:	/****** type *****/
    static void	   init();
    static pfType* getClassType() { return classType; }
    virtual const char*	getTypeName(void) const { return "pfiInputCoord"; }
  public:	/****** basic interface *****/
    virtual void	setFocus(int _focus) { focus = _focus; }
    virtual int		getFocus(void) const { return (focus != 0); }
    virtual int		inMotion(void) const { return 0; }
    virtual void	reset(void) { focus = 0; }
    virtual void	setVec(float *vec);
    virtual void	setPrev(float *vec);
    virtual void	getVec(float *vec) const;
    virtual void	getPrev(float *vec) const;
    virtual float	getCoord(int index) const;
    virtual float	getPrevCoord(int index) const;
    virtual float	getDelta(int index) const;
    virtual void	updatePrev(void);
    
		/***** parent class copy routines - don't copy what we don't know ****/
    virtual int copy(const pfMemory *_src) {return pfMemory::copy(_src);}
    virtual void copy(const pfObject *_src) {pfObject::copy(_src);}
		/***** our copy routine ****/
    void	copy(pfiInputCoord *_src) {focus = _src->focus;}
  
};

class PFUIDLLEXPORT pfi2DInputCoord : public pfiInputCoord
{
  private:
    static pfType *classType;
  public:
    pfVec2		pos;
    pfVec2		prevPos;
  public:	/****** constructor/destructors ******/
    pfi2DInputCoord(void);
  public:	/****** type *****/
    static void	   init();
    static pfType* getClassType() { return classType; }
    virtual const char*	getTypeName(void) const { return "pfi2DInputCoord"; }
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
    virtual float	getCoord(int _index) const {
	return	pos[_index];
    }
    virtual float	getPrevCoord(int _index) const {
	return	prevPos[_index];
    }
    virtual float	getDelta(int _index) const {
	if (prevPos[_index] < -1.0f)
	    return 0.0f;
	return (pos[_index] - prevPos[_index]);
    }
    virtual void	getVec(float *vec) const {
	vec[PF_X] = pos[PF_X];
	vec[PF_Y] = pos[PF_Y];
    }
    virtual void	getPrev(float *vec) const {
	vec[PF_X] = prevPos[PF_X];
	vec[PF_Y] = prevPos[PF_Y];
    }
    virtual void	setVec(float *vec) {
	updatePrev();
	PFSET_VEC2(pos, vec[PF_X], vec[PF_Y]);
    }
    virtual void	setPrev(float *vec) {
	PFSET_VEC2(prevPos, vec[PF_X], vec[PF_Y]);
    }
    virtual void	updatePrev(void) {
	PFCOPY_VEC2(prevPos, pos);
    }
};

class PFUIDLLEXPORT pfiMotionCoord : public pfObject
{
  private:
    static pfType *classType;
  public:
		/* position parameters */
    pfCoord		startPos, prevPos, pos;
    double		startTime, prevTime, time;
    pfMatrix		mat;
		/* motion parameters */
    float		speed, minSpeed, maxSpeed, startSpeed;
    float		angularVel, minAngVel, maxAngVel, startAngVel, angAccel;
    float		accel, minAccel, maxAccel, startAccel;
  public:	/* constructor/destructors */
    pfiMotionCoord(void);
  public:	/****** type *****/
    static void	   init();
    static pfType* getClassType() { return classType; }
    virtual const char*	getTypeName(void) const { return "pfiMotionCoord"; }

  public:	/***** parent class copy routines - don't copy what we don't know ****/
    virtual int copy(const pfMemory *_src) {return pfMemory::copy(_src);}
    virtual void copy(const pfObject *_src) {pfObject::copy(_src);}
  public:	/****** basic interface *****/
    void		makeMat(void);
    void		reset(void);
    void		resetPosition(void);
    void		setMat(pfMatrix& _mat);
    void		setCoord(pfCoord *_coord);
		/***** our copy routines ****/
    void		copy(pfiMotionCoord *_src);
    void		copyPos(pfiMotionCoord *_src);
    void		copyMotion(pfiMotionCoord *_src);
  public:
};

class PFUIDLLEXPORT pfiInput : public pfObject
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
    pfiEventStreamHandlerType	xEventStreamCollector;
    pfiEventStreamHandlerType	xEventStreamProcessor;
  public:	/* constructor/destructors */
    pfiInput(void);
    virtual ~pfiInput(void);
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
    void		setEventStreamProcessor(pfiEventStreamHandlerType _func, void *_data) {
		xEventStreamProcessor = _func;
		xEventStreamProcessorData = _data;
    }
    void 		getEventStreamProcessor(pfiEventStreamHandlerType *_func, void **_data) const {
	*_func = xEventStreamProcessor;
	*_data = xEventStreamProcessorData;
    }
    void		setEventStreamCollector(pfiEventStreamHandlerType _func, void *_data) {
		xEventStreamCollector = _func;
		xEventStreamCollectorData = _data;
    }
    void getEventStreamCollector(pfiEventStreamHandlerType *_func, void **_data) const {
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
    virtual const char*	getTypeName(void) const { return "pfiInput"; }
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

class PFUIDLLEXPORT pfiInputXform : public pfObject
{
  private:
    static pfType *classType;
    char		xName[PF_MAXSTRING];
  public:
    short		xFocus;
    pfiMotionCoord	xMotionCoord;
    short		xInMotion, xMotionMode, xAccelMode, xAuto;
    int			xUpdateMode;
    int			xLimitPosMask, xLimitSpeedMask, xLimitAccelMask;
    float		xDBSize; /* diameter of xDBSphere */
    float		xDBSizeLg; /* log of DBSize */
    pfBox		xDBLimits; /* tight bounding box on database */
    pfSphere		xDBSphere; /* bounding sphere for xDBLimits */
    int			xUserSetMask; /* mask of user set props, tells us not to auto set */
    pfiInput		*xInput;
    pfiInputCoord	*xInputCoord;
    void		*xMotionCBData;
    pfiInputXformFuncType	xStartMotionCB, xStopMotionCB;
    void		*xUpdateCBData;
    pfiInputXformUpdateFuncType xUpdateCB;
  public:	/****** constructor/destructors ******/
    pfiInputXform(void);
  public:	/****** type ******/
    static void	   init();
    static pfType* getClassType() { return classType; }
    virtual const char*	getTypeName(void) const { return "pfiInputXform"; }
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
    virtual void		setMat(pfMatrix& _mat);
    virtual void		getMat(pfMatrix& _mat) {
	PFCOPY_MAT(_mat,  xMotionCoord.mat);
    }
    virtual void	setInputCoordPtr(pfiInputCoord *_ic) { xInputCoord = _ic; }
    virtual pfiInputCoord* getInputCoordPtr(void) { return xInputCoord; }
    virtual void	setInput(pfiInput *_in) { xInput = _in; }
    virtual pfiInput*	getInput(void) { return xInput; }
    virtual void	setMotionCoord(pfiMotionCoord *_coord);
    virtual void	getMotionCoord(pfiMotionCoord *_coord);
    virtual void	setCoord(pfCoord *_coord);
    virtual void	getCoord(pfCoord *_coord) {
	*_coord = xMotionCoord.pos;
    }
    virtual void	setResetCoord(pfCoord *_coord) {
	xMotionCoord.startPos = *_coord;
	xUserSetMask |= _PFIX_USET_RESET_POS;
    }
    virtual void	getResetCoord(pfCoord *_coord) {
	*_coord = xMotionCoord.startPos;
    }
    virtual void	setStartSpeed(float _speed) {
	xMotionCoord.startSpeed = _speed;
    }
    virtual float	getStartSpeed(void) const {
	return xMotionCoord.startSpeed; 
    }
    virtual void	setUpdateFunc(pfiInputXformUpdateFuncType _updateCB, 
				void *_updateData) {
	xUpdateCB =_updateCB;
	xUpdateCBData = _updateData;	   
    }
    virtual void	getUpdateFunc(pfiInputXformUpdateFuncType *_updateCB, 
				void **_updateData) {
	*_updateCB = xUpdateCB;
	*_updateData = xUpdateCBData;	   
    }
    // XXX Motion callbacks and isInMotion aren't really the same thing
    virtual void	setMotionFuncs(pfiInputXformFuncType _startCB, 
				       pfiInputXformFuncType _stopCB, 
				       void *_motionData) {
	xStartMotionCB = _startCB;
	xStopMotionCB =_stopCB;
	xMotionCBData = _motionData;	   
   }
   virtual void		getMotionFuncs(pfiInputXformFuncType *_startCB, 
				       pfiInputXformFuncType *_stopCB, 
				       void **_motionData) {
	*_startCB = xStartMotionCB;
	*_stopCB = xStopMotionCB;
	*_motionData = xMotionCBData;
    }
    virtual void	setStartMotion(float _startSpeed, float _startAccel) {
	xUserSetMask |= _PFIX_USET_START_MOTION;
	xMotionCoord.startSpeed = _startSpeed;
	xMotionCoord.accel = xMotionCoord.startAccel = _startAccel;
    }
    virtual void	getStartMotion(float *_startSpeed,
				     float *_startAccel) const {
	*_startSpeed = xMotionCoord.startSpeed;
	*_startAccel = xMotionCoord.startAccel;
    }
    virtual void	setMotionLimits(float _maxSpeed,
				     float _angularVel, float _maxAccel);
    virtual void	getMotionLimits(float *_maxSpeed,
				     float *_angularVel, float *_maxAccel) const;
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
    virtual void	setCenter(float _x, float _y, float _z); /* setCenter updates BSphere */
    virtual void	getCenter(float *_x, float *_y, float *_z) const {
	*_x = xDBSphere.center[0];
	*_y = xDBSphere.center[1];
	*_z = xDBSphere.center[2];
    }
    virtual void	setDBSize(float _size); /* updates BSphere */
    virtual float	getDBSize(void) const { return xDBSize; }
};

class PFUIDLLEXPORT pfiInputXformTravel : public pfiInputXform
{
  private:
    static pfType *classType;
  public:
    int			xMotionMod;
    float		xAccelRate; /* extra acceleration/deceleration multiplier */
  public:	/****** constructor/destructors ******/
    pfiInputXformTravel(void);
  public:	/****** type *****/
    static void	   init();
    static pfType* getClassType() { return classType; }
    virtual const char*	getTypeName(void) const { return "pfiInputXformTravel"; }
  public:	/***** sets/gets *****/
    void		setAccelRate(float _m) { xAccelRate = _m; }
    float		getAccelRate(void) { return xAccelRate; }
  public: 	/**** generic interface ****/
    virtual void	setMode(int _mode, int _val);
    virtual int		getMode(int _mode) const;
    virtual void	reset(void);
    virtual void	stop(void);
};

#endif /* PF_CPLUSPLUS_API */

#endif /* __INPUTXFORM_H__ */
