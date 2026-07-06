/*
 * Copyright 1995, Silicon Graphics, Inc.
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
 *
 * file: pfiCollide.h
 * ------------------
 *
 * $REVISION:
 * $DATE:
 *
 */

#ifndef __COLLIDE_H__
#define __COLLIDE_H__

#include <Performer/pfui.h>
#include <Performer/pfui/pfiInputXform.h>

#if PF_CPLUSPLUS_API

class PFUIDLLEXPORT pfiCollide : public pfObject
{
  private:
    static pfType *classType;
  public:
    int			cEnable;
    int			cTarget;
    int			cResponse;
    int			cStatus;
    float		cDist;
    float		cHeightAboveGround;
    float		cSpeed;
    pfCoord		cPos, cPrevPos;
    pfNode*		cGroundNode;
    pfNode*		cObjNode;
    void*		cCollisionCBData;
    pfiCollideFuncType	cCollisionCB;
  public:	/****** constructor/destructors ******/
    pfiCollide(void);
    ~pfiCollide(void);
  public:	/****** type *****/
    static void	   init();
    static pfType* getClassType() { return classType; }
    virtual const char*	getTypeName(void) const { return "pfiCollide"; }
  public:	/***** sets/gets *****/
    void		setMode(int _mode, int _val);
    int			getMode(int _mode) const;
    void		setStatus(int _status) { cStatus = _status; }
    int			getStatus(void) const { return cStatus; }
    void		setDist(float _dist) { cDist = _dist; }
    float		getDist(void) const { return cDist; }
    void		setHeightAboveGrnd(float _dist) { cHeightAboveGround = _dist; }
    float		getHeightAboveGrnd(void) const { return cHeightAboveGround; }
    void		setGroundNode(pfNode* _ground) { 
	cGroundNode = _ground; 
	pfRef(cGroundNode);
    }
    pfNode*		getGroundNode(void) const { return cGroundNode; }
    void		setObjNode(pfNode* _db) { 
	cObjNode = _db; 
	pfRef(cObjNode);
    }
    pfNode*		getObjNode(void) const { return cObjNode; }
    
    void		setCurMotionParams(pfCoord *_pos, pfCoord *_prevPos, float _speed) {
	PFCOPY_VEC3(cPos.xyz, _pos->xyz);
	PFCOPY_VEC3(cPos.hpr, _pos->hpr);
	PFCOPY_VEC3(cPrevPos.xyz, _prevPos->xyz);
	PFCOPY_VEC3(cPrevPos.hpr, _prevPos->hpr);
	cSpeed = _speed;
    }
    void		getCurMotionParams(pfCoord *_pos, pfCoord *_prevPos, float *_speed) {
	PFCOPY_VEC3(_pos->xyz, cPos.xyz);
	PFCOPY_VEC3(_pos->hpr, cPos.hpr);
	PFCOPY_VEC3(_prevPos->xyz, cPrevPos.xyz);
	PFCOPY_VEC3(_prevPos->hpr, cPrevPos.hpr);
	*_speed = cSpeed;
    }
    
    void		setCollisionFunc(pfiCollideFuncType _func, void *_data) {
	cCollisionCB = _func;
	cCollisionCBData = _data;
    }
    void		getCollisionFunc(pfiCollideFuncType *_func, void **_data) {
	*_func = cCollisionCB;
	*_data = cCollisionCBData;
    }
   
  public:	/***** specific interface *****/
    void	enable(void)	{ cEnable = 1; }
    void	disable(void)	{ cEnable = cStatus = 0; }
    int		getEnable(void)	{ return cEnable; }
    int		update(void);
    void	reset(void);
    
};
#endif /* PF_CPLUSPLUS_API */
 
#endif /* __COLLIDE_H__ */
