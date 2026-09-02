/*
 * WARNING! DO NOT EDIT!
 * pfuiD.h automatically generated from 
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
 * file: pfuiD.h
 * --------------
 *
 * $Revision: 1.53 $
 * $Date: 2002/07/24 01:55:45 $
 *
 * IRIS Performer user interface library header.
 */

#ifndef __PFUI_D_H__
#define __PFUI_D_H__

#include <Performer/pfutil.h>

#ifdef __BUILD_PFUI__
#define PFUIDLLEXPORT __declspec(dllexport)
#else /*__BUILD_PFUI__ */
#define PFUIDLLEXPORT __declspec(dllimport)
#endif /*__BUILD_PFUI__ */

#ifndef PF_CPLUSPLUS_API
#ifdef __cplusplus
#define PF_CPLUSPLUS_API 1
#else
#define PF_CPLUSPLUS_API 0
#endif
#endif

#define PFI_BIGDB 1e10f

#ifdef __cplusplus
extern "C" {
#endif

#if !PF_CPLUSPLUS_API
typedef struct _pfiCollideD pfiCollideD;
typedef struct _pfiPickD pfiPickD;
typedef struct _pfiInputCoordD pfiInputCoordD;
typedef struct _pfi2DInputCoordD pfi2DInputCoordD;
typedef struct _pfiMotionCoordD pfiMotionCoordD;
typedef struct _pfiInputD pfiInputD;
typedef struct _pfiInputXformD pfiInputXformD;
typedef struct _pfiInputXformDTrackball pfiInputXformDTrackball;
typedef struct _pfiInputXformDTravel pfiInputXformDTravel;
typedef struct _pfiInputXformDDrive pfiInputXformDDrive;
typedef struct _pfiInputXformDFly pfiInputXformDFly;
typedef struct _pfiInputXformDSpheric pfiInputXformDSpheric;

#else

class pfiCollideD;
class pfiPickD;
class pfiInputCoordD;
class pfi2DInputCoordD;
class pfiMotionCoordD;
class pfiInputD;
class pfiInputXformD;
class pfiInputXformDTrackball;
class pfiInputXformDTravel;
class pfiInputXformDDrive;
class pfiInputXformDFly;
class pfiInputXformDSpheric;
#endif /* !PF_CPLUSPLUS_API */

extern PFUIDLLEXPORT void pfiInit(void);

	/*------------------------ pfiMotionCoordD ------------------------*/

extern PFUIDLLEXPORT pfType* pfiGetMotionCoordDClassType(void);
extern PFUIDLLEXPORT pfiMotionCoordD *pfiNewMotionCoordD(void *arena);

	/*------------------------ pfiInputCoordD ------------------------*/

extern PFUIDLLEXPORT pfType* pfiGetInputCoordDClassType(void);
extern PFUIDLLEXPORT pfiInputCoordD *pfiNewInputCoordD(void *arena);
extern PFUIDLLEXPORT void pfiInputCoordDVec(pfiInputCoordD *ic, double *_vec);
extern PFUIDLLEXPORT void pfiGetInputCoordDVec(pfiInputCoordD *ic, double *_vec);

	/*------------------------ pfiInputXformD ------------------------*/

typedef int (*pfiEventStreamHandlerTypeD)(pfiInputD *, pfuEventStream *,  void *);
typedef int (*pfiInputXformDFuncType)(pfiInputXformD *, void *);
typedef int (*pfiInputXformDUpdateFuncType)(pfiInputXformD *, pfiInputCoordD *, void *);


/* callback return Values */
#define PFI_CB_CONT	0
#define PFI_CB_TERM	1
#define PFI_CB_ABORT	2

extern PFUIDLLEXPORT pfiInputD *pfiNewInputD(void *arena);
extern PFUIDLLEXPORT void pfiInputDName(pfiInputD *in, const char *_name);
extern PFUIDLLEXPORT const char *pfiIsIXDGetName(pfiInputD *in);
extern PFUIDLLEXPORT void pfiInputDFocus(pfiInputD *in, int _focus);
extern PFUIDLLEXPORT int pfiGetInputDFocus(pfiInputD *in);
extern PFUIDLLEXPORT void pfiInputDEventMask(pfiInputD *in, int _emask);
extern PFUIDLLEXPORT int pfiGetInputDEventMask(pfiInputD *in);
extern PFUIDLLEXPORT void pfiInputDEventStreamCollector(pfiInputD *in, pfiEventStreamHandlerTypeD _func, void *data);
extern PFUIDLLEXPORT void pfiGetInputDEventStreamCollector(pfiInputD *in, pfiEventStreamHandlerTypeD *_func, void **data);
extern PFUIDLLEXPORT void pfiInputDEventStreamProcessor(pfiInputD *in, pfiEventStreamHandlerTypeD _func, void *data);
extern PFUIDLLEXPORT void pfiGetInputDEventStreamProcessor(pfiInputD *in, pfiEventStreamHandlerTypeD *_func, void **data);
extern PFUIDLLEXPORT void pfiInputDEventHandler(pfiInputD *in, pfuEventHandlerFuncType _func, void *data);
extern PFUIDLLEXPORT void pfiGetInputDEventHandler(pfiInputD *in, pfuEventHandlerFuncType *_func, void **data);
extern PFUIDLLEXPORT void pfiResetInputD(pfiInputD *_in);
extern PFUIDLLEXPORT void pfiCollectInputDEvents(pfiInputD *_in);
extern PFUIDLLEXPORT void pfiProcessInputDEvents(pfiInputD *_in);

extern PFUIDLLEXPORT int pfiHaveFastMouseClickD(pfuMouse *mouse, int button, double msecs);

extern PFUIDLLEXPORT pfiInputXformD *pfiNewIXformD(void *arena);
extern PFUIDLLEXPORT void pfiIXformDFocus(pfiInputXformD *in, int _focus);
extern PFUIDLLEXPORT int pfiIsIXDformInMotion(pfiInputXformD *_ix);
extern PFUIDLLEXPORT void pfiStopIXformD(pfiInputXformD *_ix);
extern PFUIDLLEXPORT void pfiResetIXformD(pfiInputXformD *_ix);
extern PFUIDLLEXPORT void pfiUpdateIXformD(pfiInputXformD *_ix);
extern PFUIDLLEXPORT void pfiIXformDMode(pfiInputXformD *_ix, int mode, int val);
extern PFUIDLLEXPORT int pfiGetIXformDMode(pfiInputXformD *_ix, int mode);
extern PFUIDLLEXPORT void pfiResetIXformDPosition(pfiInputXformD *_ix);
extern PFUIDLLEXPORT void pfiIXformDMat(pfiInputXformD *_ix, PFMATRIX4D _mat);
extern PFUIDLLEXPORT void pfiGetIXformDMat(pfiInputXformD *_ix, PFMATRIX4D _mat);
extern PFUIDLLEXPORT void pfiIXformDInputD(pfiInputXformD *_ix, pfiInputD *_in);
extern PFUIDLLEXPORT pfiInputD* pfiGetIXformDInputD(pfiInputXformD *_ix);
extern PFUIDLLEXPORT void pfiIXformDInputCoordDPtr(pfiInputXformD *_ix, pfiInputCoordD *_xcoord);
extern PFUIDLLEXPORT pfiInputCoordD* pfiGetIXformDInputCoordDPtr(pfiInputXformD *_ix);
extern PFUIDLLEXPORT void pfiIXformDMotionCoordD(pfiInputXformD *_ix, pfiMotionCoordD *_xcoord);
extern PFUIDLLEXPORT void pfiGetIXformDMotionCoordD(pfiInputXformD *_ix, pfiMotionCoordD *_xcoord);
extern PFUIDLLEXPORT void pfiIXformDResetCoord(pfiInputXformD *_ix, pfCoordd *_resetPos);
extern PFUIDLLEXPORT void pfiGetIXformDResetCoord(pfiInputXformD *_ix, pfCoordd *_resetPos);
extern PFUIDLLEXPORT void pfiIXformDCoord(pfiInputXformD *_ix, pfCoordd *_coord);
extern PFUIDLLEXPORT void pfiGetIXformDCoord(pfiInputXformD *_ix, pfCoordd *_coord);
extern PFUIDLLEXPORT void pfiIXformDStartMotion(pfiInputXformD *_xf, double _startSpeed, double _startAccel);
extern PFUIDLLEXPORT void pfiGetIXformDStartMotion(pfiInputXformD *_xf, double *_startSpeed, double *_startAccel);
extern PFUIDLLEXPORT void pfiIXformDMotionLimits(pfiInputXformD *_xf, double _maxSpeed, double _angularVel, double _maxAccel);
extern PFUIDLLEXPORT void pfiGetIXformDMotionLimits(pfiInputXformD *_xf, double *_maxSpeed, double *_angularVel, double *_maxAccel);
extern PFUIDLLEXPORT void pfiIXformDDBLimits(pfiInputXformD *_xf, pfBox *_dbLimits);
extern PFUIDLLEXPORT void pfiGetIXformDDBLimits(pfiInputXformD *_xf, pfBox *_dbLimits);
extern PFUIDLLEXPORT void pfiIXformDBSphere(pfiInputXformD *_xf, pfSphere *_sphere);
extern PFUIDLLEXPORT void pfiGetIXformDBSphere(pfiInputXformD *_xf, pfSphere *_sphere);
extern PFUIDLLEXPORT void pfiIXformDUpdateFunc(pfiInputXformD *_ix, pfiInputXformDUpdateFuncType _func, void *_data);
extern PFUIDLLEXPORT void pfiGetIXformDUpdateFunc(pfiInputXformD *_ix, pfiInputXformDUpdateFuncType *_func, void **_data);
extern PFUIDLLEXPORT void pfiIXformDMotionFuncs(pfiInputXformD *_ix, pfiInputXformDFuncType _start, pfiInputXformDFuncType _stop, void *_data);
extern PFUIDLLEXPORT void pfiGetIXformDMotionFuncs(pfiInputXformD *_ix, pfiInputXformDFuncType *_start, pfiInputXformDFuncType *_stop, void **_data);

/* General InputXformD Modes to be accepted by all */
#define PFIX_MODE_MOTION	0
#define PFIX_MODE_MOTION_MOD	1 /* MOTION_MOD bitmask */
#define PFIX_MODE_ACCEL		2
#define PFIX_MODE_AUTO		3
#define PFIX_MODE_LIMIT_POS	4 /* PFIX_LIMIT_POS bitmask */
#define PFIX_MODE_LIMIT_SPEED	5 /* PFIX_LIMIT_SPEED bitmask */
#define PFIX_MODE_LIMIT_ACCEL	6 /* PFIX_LIMIT_ACCEL bitmask */

/* InputXformD Mode values */
#define PFIX_MOTION_STOP	0
#define PFIX_ACCEL_NONE	0
#define PFIX_LIMIT_POS_NONE	0x0
#define PFIX_LIMIT_POS_HORIZ	0x1
#define PFIX_LIMIT_POS_BOTTOM	0x2
#define PFIX_LIMIT_POS_TOP	0x4
#define PFIX_LIMIT_SPEED_NONE	0x0
#define PFIX_LIMIT_SPEED_MAX	0x1
#define PFIX_LIMIT_SPEED_DB	0x2
#define PFIX_LIMIT_ACCEL_NONE	0x0
#define PFIX_LIMIT_ACCEL_MAX	0x1
#define PFIX_LIMIT_ACCEL_DB	0x2

/* Trackball modes */
#define PFIXTK_MODE_MOTION	PFIX_MODE_MOTION
#define PFIXTK_MODE_MOTION_MOD	PFIX_MODE_MOTION_MOD
#define PFIXTK_MODE_ACCEL	PFIX_MODE_ACCEL
#define PFIXTK_MODE_AUTO	PFIX_MODE_AUTO

/* Trackball motion modes */
#define PFIXTK_MOTION_STOP	PFIX_MOTION_STOP
#define PFIXTK_MOTION_ROTY	100
#define PFIXTK_MOTION_ROTXZ	110
#define PFIXTK_MOTION_TRANSXZ	120
#define PFIXTK_MOTION_TRANSY	130

/* Trackball acceleration modes */
#define PFIXTK_ACCEL_NONE	PFIX_ACCEL_NONE
#define PFIXTK_ACCEL_INCREASE	100
#define PFIXTK_ACCEL_DECREASE	110

extern PFUIDLLEXPORT pfiInputXformDTrackball *pfiNewIXformDTrackball(void *_arena);
extern PFUIDLLEXPORT void pfiIXformDTrackballMode(pfiInputXformDTrackball *_tb, int _mode, int _val);
extern PFUIDLLEXPORT int  pfiGetIXformDTrackballMode(pfiInputXformDTrackball *_tb, int _mode);
extern PFUIDLLEXPORT pfiInputXformDTrackball *pfiCreate2DIXformDTrackball(void *arena);
extern PFUIDLLEXPORT int pfiUpdate2DIXformDTrackball(pfiInputXformD *_tb, pfiInputCoordD *_icoord, void *data);

    /*  ------ SPHERIC DATA ------ */
/* Spheric modes */
#define PFIXSPH_MODE_MOTION	PFIX_MODE_MOTION
#define PFIXSPH_MODE_MOTION_MOD	PFIX_MODE_MOTION_MOD
#define PFIXSPH_MODE_AUTO	PFIX_MODE_AUTO

/* Spheric motion mods */
#define PFIXSPH_MOTION_STOP      PFIX_MOTION_STOP
#define PFIXSPH_MOTION_AUTO      PFIX_MOTION_AUTO
#define PFIXSPH_MOTION_INOUT     310
#define PFIXSPH_MOTION_AROUND    311
#define PFIXSPH_MOTION_TRACK_INOUT 312

/* Spheric motion modifier bits */
#define PFIXSPH_MOTION_MOD_SPIN      0x0001

/* Spheric motion parameters */
#define PFIXSPH_RADIUS 901
#define PFIXSPH_MIN_RADIUS 902
#define PFIXSPH_MAX_RADIUS 903

#define PFIXSPH_AROUND 904
#define PFIXSPH_NEAR_MIN_AROUND 905
#define PFIXSPH_NEAR_MAX_AROUND 906
#define PFIXSPH_FAR_MIN_AROUND 907
#define PFIXSPH_FAR_MAX_AROUND 908

#define PFIXSPH_TILT 909
#define PFIXSPH_NEAR_MIN_TILT 910
#define PFIXSPH_NEAR_MAX_TILT 911
#define PFIXSPH_FAR_MIN_TILT 912
#define PFIXSPH_FAR_MAX_TILT 913

#define PFIXSPH_ROLL 914
#define PFIXSPH_NEAR_MIN_ROLL 915
#define PFIXSPH_NEAR_MAX_ROLL 916
#define PFIXSPH_FAR_MIN_ROLL 917
#define PFIXSPH_FAR_MAX_ROLL 918

#define PFIXSPH_NEAR_MAX_LINEAR_VELOCITY 919
#define PFIXSPH_NEAR_MAX_ANGULAR_VELOCITY 920
#define PFIXSPH_NEAR_MAX_ROLL_VELOCITY 921
#define PFIXSPH_FAR_MAX_LINEAR_VELOCITY 922
#define PFIXSPH_FAR_MAX_ANGULAR_VELOCITY 923
#define PFIXSPH_FAR_MAX_ROLL_VELOCITY 924

#define PFIXSPH_CENTER_X 925
#define PFIXSPH_CENTER_Y 926
#define PFIXSPH_CENTER_Z 927

#define PFIXSPH_INT_NUM_CHILDREN 928
#define PFIXSPH_INT_AT_RADIUS 929
#define PFIXSPH_INT_AT_AROUND 930
#define PFIXSPH_INT_AT_TILT 931

#define PFIXSPH_INT_CHILD_NUM 932

#define PFIXSPH_OUT_RADIUS  1000
#define PFIXSPH_IN_RADIUS_0 1001
#define PFIXSPH_IN_RADIUS_1 1002

#define PFIXSPH_OUT_AROUND  1100
#define PFIXSPH_IN_AROUND_0 1101
#define PFIXSPH_IN_AROUND_1 1102

#define PFIXSPH_OUT_TILT  1200
#define PFIXSPH_IN_TILT_0 1201
#define PFIXSPH_IN_TILT_1 1202

/* Spheric motion path stuff */
#define PFIXSPH_PATH_IN 1
#define PFIXSPH_PATH_OUT 2
#define PFIXSPH_PATH_UNKNOWN 3

#define PFIXSPH_MAX_CHILDREN 2

extern PFUIDLLEXPORT pfiInputXformDSpheric *pfiNewIXSphericD(void *arena);
extern PFUIDLLEXPORT void pfiIXformDSphericMode(pfiInputXformDSpheric *spheric, int mode,
				 int val);
extern PFUIDLLEXPORT int pfiGetIXformDSphericMode(pfiInputXformDSpheric *spheric, int mode);
extern PFUIDLLEXPORT void pfiIXformDSphericParameter(pfiInputXformDSpheric *spheric,
				      int _param, double _val);
extern PFUIDLLEXPORT double pfiGetIXformDSphericParameter(pfiInputXformDSpheric *spheric,
					  int _param);
extern PFUIDLLEXPORT pfiInputXformDSpheric *pfiCreate2DIXformDSpheric(void *arnea);
extern PFUIDLLEXPORT int pfiUpdate2DIXformDSpheric(pfiInputXformD *spheric,
				    pfiInputCoordD *_icoord, void *data);

void PFUIDLLEXPORT pfiIXformDSphericSetWorld(pfiInputXformDSpheric *spheric, int worldNumber,
			      int in_or_out);
void PFUIDLLEXPORT pfiIXformDSphericReadPathFile(pfiInputXformDSpheric *spheric,
				  char *filename);
void PFUIDLLEXPORT pfiIXformDSphericPrintPathStuff(pfiInputXformDSpheric *spheric);

/* Travel modes */
#define PFIXTR_MODE_MOTION	PFIX_MODE_MOTION
#define PFIXTR_MODE_MOTION_MOD	PFIX_MODE_MOTION_MOD
#define PFIXTR_MODE_ACCEL	PFIX_MODE_ACCEL
#define PFIXTR_MODE_AUTO	PFIX_MODE_AUTO

/* Travel motion modes */
#define PFIXTR_MOTION_STOP	PFIX_MOTION_STOP
#define PFIXTR_MOTION_AUTO	PFIX_MOTION_AUTO
#define PFIXTR_MOTION_FORWARD	210
#define PFIXTR_MOTION_REVERSE	211
#define PFIXTR_MOTION_DIRECTION	212

/* Travel motion modifier bits */
#define PFIXTR_MOTION_MOD_VERTICAL	0x0001


/* Travel acceleration modes */
#define PFIXTR_ACCEL_NONE	PFIX_ACCEL_NONE
#define PFIXTR_ACCEL_INCREASE	220
#define PFIXTR_ACCEL_DECREASE	221

extern PFUIDLLEXPORT pfType *pfiGetIXformDTravelClassType(void);
extern PFUIDLLEXPORT pfType *pfiGetIXformDDriveClassType(void);
extern PFUIDLLEXPORT pfType *pfiGetIXformDFlyClassType(void);
extern PFUIDLLEXPORT pfType *pfiGetIXformDSphericClassType(void);
extern PFUIDLLEXPORT pfType *pfiGetIXformDTrackballClassType(void);


/* Drive modes */
#define PFIXDR_MODE_MOTION	PFIXTR_MODE_MOTION
#define PFIXDR_MODE_MOTION_MOD	PFIXTR_MODE_MOTION_MOD
#define PFIXDR_MODE_ACCEL	PFIXTR_MODE_ACCEL
#define PFIXDR_MODE_AUTO	PFIXTR_MODE_AUTO

/* Drive motion modes */
#define PFIXDR_MOTION_STOP	PFIXTR_MOTION_STOP
#define PFIXDR_MOTION_AUTO	PFIXTR_MOTION_AUTO
#define PFIXDR_MOTION_FORWARD	PFIXTR_MOTION_FORWARD
#define PFIXDR_MOTION_REVERSE	PFIXTR_MOTION_REVERSE
#define PFIXDR_MOTION_HEADING	PFIXTR_MOTION_DIRECTION

/* Travel motion modifiers */
#define PFIXDR_MOTION_MOD_VERTICAL	PFIXTR_MOTION_MOD_VERTICAL

/* Drive acceleration modes */
#define PFIXDR_ACCEL_NONE	PFIX_ACCEL_NONE
#define PFIXDR_ACCEL_INCREASE	PFIXTR_ACCEL_INCREASE
#define PFIXDR_ACCEL_DECREASE	PFIXTR_ACCEL_DECREASE

extern PFUIDLLEXPORT pfiInputXformDDrive *pfiNewIXformDDrive(void *arena);
extern PFUIDLLEXPORT void pfiIXformDDriveMode(pfiInputXformDDrive *_drive, int mode, int val);
extern PFUIDLLEXPORT int  pfiGetIXformDDriveMode(pfiInputXformDDrive *_drive, int mode);
extern PFUIDLLEXPORT void pfiIXformDDriveHeight(pfiInputXformDDrive* _drive, double height);
extern PFUIDLLEXPORT double pfiGetIXformDDriveHeight(pfiInputXformDDrive* _drive);
extern PFUIDLLEXPORT pfiInputXformDDrive *pfiCreate2DIXformDDrive(void *arena);
extern PFUIDLLEXPORT int pfiUpdate2DIXformDDrive(pfiInputXformD *_drive, pfiInputCoordD *_icoord, void *data);

/* Fly modes */
#define PFIXFLY_MODE_MOTION	PFIXTR_MODE_MOTION
#define PFIXFLY_MODE_MOTION_MOD	PFIXTR_MODE_MOTION_MOD
#define PFIXFLY_MODE_ACCEL	PFIXTR_MODE_ACCEL
#define PFIXFLY_MODE_AUTO	PFIXTR_MODE_AUTO
#define PFIXFLY_MODE_PITCH	310

/* Fly motion modes */
#define PFIXFLY_MOTION_STOP	PFIXTR_MOTION_STOP
#define PFIXFLY_MOTION_AUTO	PFIXTR_MOTION_AUTO
#define PFIXFLY_MOTION_FORWARD	PFIXTR_MOTION_FORWARD
#define PFIXFLY_MOTION_REVERSE	PFIXTR_MOTION_REVERSE
#define PFIXFLY_MOTION_HP	PFIXTR_MOTION_DIRECTION

/* Fly motion modifiers */
#define PFIXFLY_MOTION_MOD_VERTICAL	PFIXTR_MOTION_MOD_VERTICAL


/* Fly acceleration modes */
#define PFIXFLY_ACCEL_NONE	PFIX_ACCEL_NONE
#define PFIXFLY_ACCEL_INCREASE	PFIXTR_ACCEL_INCREASE
#define PFIXFLY_ACCEL_DECREASE	PFIXTR_ACCEL_DECREASE

/* Fly pitch modes */
#define PFIXFLY_PITCH_FOLLOW	330 /* default: pitch "follows" pointer on screen */
#define PFIXFLY_PITCH_FLIGHT	331 /* true flight stick model:
				   *   pull back (mouse down) to pull nose up
				   *   push forward (mouse up) to push nose down
				   */

extern PFUIDLLEXPORT pfiInputXformDFly *pfiNewIXformFlyD(void *arena);
extern PFUIDLLEXPORT void pfiIXformDFlyMode(pfiInputXformDFly *_fly, int mode, int val);
extern PFUIDLLEXPORT int  pfiGetIXformDFlyMode(pfiInputXformDFly *_fly, int mode);
extern PFUIDLLEXPORT pfiInputXformDFly *pfiCreate2DIXformDFly(void *arnea);
extern PFUIDLLEXPORT int pfiUpdate2DIXformDFly(pfiInputXformD *_fly, pfiInputCoordD *_icoord, void *data);

	/*------------------------ pfiCollideD --------------------------*/

typedef int (*pfiCollideDFuncType)(pfiCollideD *, void *);

/* Collide Modes */
#define PFIC_TARGET		0
#define PFIC_RESPONSE		1

/* Collide Targets */
#define PFIC_TARGET_NONE	0x0
#define PFIC_TARGET_GROUND	0x1
#define PFIC_TARGET_OBJ		0x2
#define PFIC_TARGET_ALL		0x3

/* Collision responses */
#define PFIC_RESPONSE_NONE	0
#define PFIC_RESPONSE_BOUNCE	1
#define PFIC_RESPONSE_STOP	2

extern PFUIDLLEXPORT pfType *pfiGetCollideDClassType(void);

extern PFUIDLLEXPORT pfiCollideD * pfiNewCollideD(void *_arena);
extern PFUIDLLEXPORT void pfiEnableCollideD(pfiCollideD *collide);
extern PFUIDLLEXPORT void pfiDisableCollideD(pfiCollideD *collide);
extern PFUIDLLEXPORT int pfiGetCollideDEnable(pfiCollideD *collide);
extern PFUIDLLEXPORT void pfiCollideDMode(pfiCollideD *collide, int _mode, int _val);
extern PFUIDLLEXPORT int pfiGetCollideDMode(pfiCollideD *collide, int _mode);
extern PFUIDLLEXPORT void pfiCollideDStatus(pfiCollideD *collide, int _status);
extern PFUIDLLEXPORT int pfiGetCollideDStatus(pfiCollideD *collide);
extern PFUIDLLEXPORT void pfiCollideDDist(pfiCollideD *collide, double _dist);
extern PFUIDLLEXPORT double pfiGetCollideDDist(pfiCollideD *collide);
extern PFUIDLLEXPORT void pfiCollideDHeightAboveGrnd(pfiCollideD *collide, double _dist);
extern PFUIDLLEXPORT double pfiGetCollideDHeightAboveGrnd(pfiCollideD *collide);
extern PFUIDLLEXPORT void pfiCollideDGroundNode(pfiCollideD *collide, pfNode* _ground);
extern PFUIDLLEXPORT pfNode * pfiGetCollideDGroundNode(pfiCollideD *collide);
extern PFUIDLLEXPORT void pfiCollideDObjNode(pfiCollideD *collide, pfNode* _db);
extern PFUIDLLEXPORT pfNode * pfiGetCollideDObjNode(pfiCollideD *collide);
extern PFUIDLLEXPORT void pfiCollideDCurMotionParams(pfiCollideD *collide, pfCoordd* _pos, pfCoordd* _prevPos, double _speed);
extern PFUIDLLEXPORT void pfiGetCollideDCurMotionParams(pfiCollideD *collide, pfCoordd* _pos, pfCoordd* _prevPos, double *_speed);
extern PFUIDLLEXPORT void pfiGetCollideDMotionCoordD(pfiCollideD *collide, pfiMotionCoordD* _xcoord);
extern PFUIDLLEXPORT void pfiCollideDFunc(pfiCollideD *collide, pfiCollideDFuncType _func, void *_data);
extern PFUIDLLEXPORT void pfiGetCollisionFuncD(pfiCollideD *collide, pfiCollideDFuncType *_func, void **_data);
extern PFUIDLLEXPORT int pfiUpdateCollideD(pfiCollideD *collide);

	/*---------------------------- pfiPickD -----------------------------*/

/* Pick Modes */
#define PFIP_MODE_PICK		1 /* PFPK_M_NEAREST or PFPK_M_ALL */
#define PFIP_MODE_ISECT		2 /* PFTRAV_IS_* bitmasks */

typedef int (*pfiPickDFuncType)(pfiPickD *, void *);

extern PFUIDLLEXPORT pfType *pfiGetPickDClassType(void);

extern PFUIDLLEXPORT pfiPickD *	 pfiNewPickD(void *arena);
extern PFUIDLLEXPORT void		 pfiPickDMode(pfiPickD *pick, int _mode, int _val);
extern PFUIDLLEXPORT int		 pfiGetPickDMode(pfiPickD *pick, int _mode);
extern PFUIDLLEXPORT void		 pfiPickDHitFunc(pfiPickD *pick, pfiPickDFuncType _func, void *_data);
extern PFUIDLLEXPORT void		 pfiGetPickDtHitFunc(pfiPickD *pick, pfiPickDFuncType *_func, void **_data);
extern PFUIDLLEXPORT void		 pfiAddPickDChan(pfiPickD *pick, pfChannel *_chan);
extern PFUIDLLEXPORT void		 pfiInsertPickDChan(pfiPickD *pick, int _index, pfChannel *_chan);
extern PFUIDLLEXPORT void		 pfiRemovePickDChan(pfiPickD *pick, pfChannel *_chan);
extern PFUIDLLEXPORT int		 pfiGetPickDNumHits(pfiPickD *pick);
extern PFUIDLLEXPORT pfNode *		 pfiGetPickDNode(pfiPickD *pick);
extern PFUIDLLEXPORT pfGeoSet *	 pfiGetPickDGSet(pfiPickD *pick);
extern PFUIDLLEXPORT void		 pfiSetupPickDChans(pfiPickD *pick);
extern PFUIDLLEXPORT int		 pfiDoPickD(pfiPickD *pick, int _x, int _y);
extern PFUIDLLEXPORT void		 pfiResetPickD(pfiPickD *pick);

	/*------------------------ pfiXformerD --------------------------*/

#if !PF_CPLUSPLUS_API
typedef struct _pfiXformerD 	pfiXformerD;
#else
class pfiXformerD;
#endif

/* XFormer InputD Models */
#define PFIXF_INPUT_MOUSE_POLLED 200
#define PFIXF_INPUT_EVENTS	 210

extern PFUIDLLEXPORT pfType* pfiGetXformerDClassType(void);
extern PFUIDLLEXPORT pfiXformerD * pfiNewXformerD(void* arena);
extern PFUIDLLEXPORT void pfiXformerDModel(pfiXformerD* xf, int _index, pfiInputXformD* _model);
extern PFUIDLLEXPORT void pfiSelectXformerDModel(pfiXformerD* xf, int which);
extern PFUIDLLEXPORT pfiInputXformD* pfiGetXformerDCurModel(pfiXformerD* xf);
extern PFUIDLLEXPORT int pfiGetXformerDCurModelIndex(pfiXformerD* xf);
extern PFUIDLLEXPORT int pfiRemoveXformerDModel(pfiXformerD* xf, int _index);
extern PFUIDLLEXPORT int pfiRemoveXformerDModelIndex(pfiXformerD* xf, pfiInputXformD* _model);
extern PFUIDLLEXPORT void pfiStopXformerD(pfiXformerD* xf);
extern PFUIDLLEXPORT void pfiResetXformerD(pfiXformerD* xf);
extern PFUIDLLEXPORT void pfiResetXformerDPosition(pfiXformerD* xf);
extern PFUIDLLEXPORT void pfiCenterXformerD(pfiXformerD* xf);
extern PFUIDLLEXPORT void pfiXformerDAutoInput(pfiXformerD* xf, pfChannel* chan, pfuMouse* mouse, pfuEventStream* events);

extern PFUIDLLEXPORT void pfiXformerDMat(pfiXformerD* xf, PFMATRIX4D mat);
extern PFUIDLLEXPORT void pfiGetXformerDMat(pfiXformerD* xf, PFMATRIX4D mat);
extern PFUIDLLEXPORT void pfiXformerDModelMat(pfiXformerD* xf, PFMATRIX4D mat);
extern PFUIDLLEXPORT void pfiGetXformerDModelMat(pfiXformerD* xf, PFMATRIX4D mat);

extern PFUIDLLEXPORT void pfiXformerDCoord(pfiXformerD* xf, pfCoordd *coord);
extern PFUIDLLEXPORT void pfiGetXformerDCoord(pfiXformerD* xf, pfCoordd *coord);
extern PFUIDLLEXPORT void pfiXformerDResetCoord(pfiXformerD* xf, pfCoordd *_resetPos);
extern PFUIDLLEXPORT void pfiGetXformerDResetCoord(pfiXformerD* xf, pfCoordd *_resetPos);
extern PFUIDLLEXPORT void pfiXformerDNode(pfiXformerD* xf, pfNode *_node);
extern PFUIDLLEXPORT pfNode *pfiGetXformerDNode(pfiXformerD* xf);
extern PFUIDLLEXPORT void pfiXformerDAutoPosition(pfiXformerD* xf, pfChannel *_chan, pfDoubleDCS *_dcs);
extern PFUIDLLEXPORT void pfiGetXformerDAutoPosition(pfiXformerD* xf, pfChannel **_chan, pfDoubleDCS **_dcs);
extern PFUIDLLEXPORT void pfiXformerDLimits(pfiXformerD* xf, double maxSpeed, double angularVel, double maxAccel, pfBox* dbLimits);
extern PFUIDLLEXPORT void pfiGetXformerDLimits(pfiXformerD* xf, double *maxSpeed, double *angularVel, double *maxAccel, pfBox* dbLimits);
extern PFUIDLLEXPORT void pfiEnableXformerDCollision(pfiXformerD* xf);
extern PFUIDLLEXPORT void pfiDisableXformerDCollision(pfiXformerD* xf);
extern PFUIDLLEXPORT int pfiGetXformerDCollisionEnable(pfiXformerD* xf);
extern PFUIDLLEXPORT void pfiXformerDCollision(pfiXformerD* xf, int mode, double val, pfNode* node);
extern PFUIDLLEXPORT int pfiGetXformerDCollisionStatus(pfiXformerD* xf);
extern PFUIDLLEXPORT void pfiUpdateXformerD(pfiXformerD* xf);
extern PFUIDLLEXPORT int pfiCollideXformerD(pfiXformerD* xf);

/* Trackball-Drive-Fly XformerD (TDF) */


#if !PF_CPLUSPLUS_API
typedef struct _pfiTDFXformerD 	pfiTDFXformerD;
#else
class pfiTDFXformerD;
#endif

/* TDF XFormer InputD Models - indices for set/select of model for the
 * first three models
 */
#define PFITDF_TRACKBALL	    0
#define PFITDF_FLY		    1
#define PFITDF_DRIVE		    2
#define PFITDF_SPHERIC		    3

extern PFUIDLLEXPORT pfType* pfiGetTDFXformerDClassType(void);
extern PFUIDLLEXPORT pfiTDFXformerD * pfiNewTDFXformerD(void* arena);
extern PFUIDLLEXPORT pfiXformerD * pfiCreateTDFXformerD( pfiInputXformDTrackball *_tb, pfiInputXformDDrive *_drive, pfiInputXformDFly *_fly, void *arena);
extern PFUIDLLEXPORT void pfiTDFXformerDFastClickTime(pfiTDFXformerD* xf, double _time);
extern PFUIDLLEXPORT double pfiGetTDFXformerDFastClickTime(pfiXformerD* xf);
extern PFUIDLLEXPORT void pfiTDFXformerDTrackball(pfiTDFXformerD *_xf, pfiInputXformDTrackball *_tb);
extern PFUIDLLEXPORT pfiInputXformDTrackball *pfiGetTDFXformerDTrackball(pfiTDFXformerD *_xf);
extern PFUIDLLEXPORT void pfiTDFXformerDDrive(pfiTDFXformerD *_xf, pfiInputXformDDrive *_tb);
extern PFUIDLLEXPORT pfiInputXformDFly *pfiGetTDFXformerDFly(pfiTDFXformerD *_xf);
extern PFUIDLLEXPORT void pfiTDFXformerDFly(pfiTDFXformerD *_xf, pfiInputXformDFly *_tb);
extern PFUIDLLEXPORT pfiInputXformDDrive *pfiGetTDFXformerDDrive(pfiTDFXformerD *_xf);
extern PFUIDLLEXPORT void pfiTDFXformerDSpheric(pfiTDFXformerD *_xf, pfiInputXformDSpheric *_tb);
extern PFUIDLLEXPORT pfiInputXformDSpheric *pfiGetTDFXformerDSpheric(pfiTDFXformerD *_xf);

/* these routines implement the TDF mouse-based user-interface */
extern PFUIDLLEXPORT int pfiProcessTDFXformerDMouseEvents(pfiInputD *, pfuEventStream *,  void *data);
extern PFUIDLLEXPORT void pfiProcessTDFXformerDMouse(pfiTDFXformerD *xf, pfuMouse *mouse, pfChannel *inputChan);
extern PFUIDLLEXPORT void pfiProcessTDFTrackballDMouse(pfiTDFXformerD *xf, pfiInputXformDTrackball *trackball, pfuMouse *mouse);
extern PFUIDLLEXPORT void pfiProcessTDFSphericDMouse(pfiTDFXformerD *xf, pfiInputXformDSpheric *spheric, pfuMouse *mouse);
extern PFUIDLLEXPORT void pfiProcessTDFTravelDMouse(pfiTDFXformerD *xf, pfiInputXformDTravel *tr, pfuMouse *mouse);

	/* ----------------- Macros for 1.2 Compatibility ----------------- */

#ifdef PF_COMPAT_1_2

typedef struct pfiXformerD 		pfuXformerD;

#define PFUXF_TRACKBALL			0
#define PFUXF_DRIVE			1
#define PFUXF_FLY			2

#define pfuNewXformerD(_a)		pfiNewTDFXformerD(_a)
#define pfuXformerDMode(_xf, _m)		pfiSelectXformerDModel(_xf, _m)
#define pfuGetXformerDMode(_xf)		pfiGetXformerDCurModelIndex(_xf)
#define pfuStopXformerD(_xf)		pfiStopXformerD(_xf)
#define pfuXformerDAutoInput(_xf, _chan, _mouse, _events) \
		pfiXformerDAutoInput(_xf, _chan, _mouse, _events)
#define pfuXformerDMat(_xf, _mat)    pfiXformerDMat(_xf, _mat)
#define pfuGetXformerDMat(_xf, _mat) pfiGetXformerDMat(_xf, _mat)
#define pfuXformerDCoord(_xf, _coord)	pfiXformerDCoord(_xf, _coord)
#define pfuGetXformerDCoord(_xf, _coord)	pfiGetXformerDCoord(_xf, _coord)
#define pfuXformerDLimits(_xf, _maxSpeed, _angularVel, _maxAccel, _dbLimits) \
		pfiXformerDLimits(_xf, _maxSpeed, _angularVel, _maxAccel, _dbLimits)
#define pfuGetXformerDLimits(_xf, _maxSpeed, _angularVel, _maxAccel, _dbLimits) \
		pfiGetXformerDLimits(_xf, _maxSpeed, _angularVel, _maxAccel, _dbLimits)
#define pfuXformerDCollision(_xf, _mode, _val,_node) \
		pfiXformerDCollision(_xf, _mode, _val,_node)
#define pfuGetXformerDCollisionStatus(_xf) \
		pfiGetXformerDCollisionStatus(_xf)
#define pfuUpdateXformerD(_xf)		pfiUpdateXformerD(_xf)
#define pfuCollideXformerD(_xf)		pfiCollideXformerD(_xf)

#endif /* PF_COMPAT_1_2 */

	/* ------------------- Macros for typecasting --------------------- */


#if defined(__STDC__) || defined(__cplusplus)

#ifndef __PFI_INPUTXFORM_C__

#define pfiInputDName(_ix, _name) \
	    pfiInputDName((pfiInputD *)_ix, _name)
#define pfiGetInputDName(_ix) \
	    pfiGetInputDName((pfiInputD *)_ix)
#define pfiIsInputDInMotion(_ix) \
	    pfiIsInputDInMotion(_ix)
#define pfiInputDFocus(_ix, _focus) \
	    pfiInputDFocus((pfiInputD *)_ix, _focus)
#define pfiGetInputDFocus(_ix) \
	    pfiGetInputDFocus((pfiInputD *)_ix)
#define pfiInputDEventMask(_ix, _emask) \
	    pfiInputDEventMask((pfiInputD *)_ix, _emask)
#define pfiGetInputDEventMask(_ix) \
	    pfiGetInputDEventMask((pfiInputD *)_ix)
#define pfiInputDEventStreamCollector(_ix, _func) \
	    pfiInputDEventStreamCollector(_ix, _func)
#define pfiGetInputDEventStreamCollector(_ix, _func, _data) \
	    pfiGetInputDEventStreamCollector((pfiInputD *)_ix, _func, _data)
#define pfiInputDEventStreamProcessor(_ix, _func) \
	    pfiInputDEventStreamProcessor(_ix, _func)
#define pfiGetInputDEventStreamProcessor(_ix, _func, _data) \
	    pfiGetInputDEventStreamProcessor((pfiInputD *)_ix, _func, _data)
#define pfiInputDEventHandler(_ix, _func) \
	    pfiInputDEventHandler(_ix, _func)
#define pfiGetInputDEventHandler(_ix, _func, _data) \
	    pfiGetInputDEventHandler((pfiInputD *)_ix, _func, _data)
#define pfiResetInputD(_ix) \
	    pfiResetInputD((pfiInputD *)_ix)
#define pfiCollectInputDEvents(_ix) \
	    pfiCollectInputDEvents((pfiInputD *)_ix)
#define pfiProcessInputDEvents(_ix) \
	    pfiProcessInputDEvents((pfiInputD *)_ix)

#define pfiInputCoordDVec(_ic, _vec) \
	    pfiInputCoordDVec((pfiInputCoordD*)_ic, _vec)
#define pfiGetInputCoordDVec(_ic, _vec) \
	    pfiGetInputCoordDVec((pfiInputCoordD*)_ic, _vec)

#define pfiIXformDFocus(_ix, _focus) \
	    pfiIXformDFocus((pfiInputXformD *)_ix, _focus)
#define pfiStopIXformD(_ix) \
	    pfiStopIXformD((pfiInputXformD *)_ix)
#define pfiResetIXformD(_ix) \
	    pfiResetIXformD((pfiInputXformD *)_ix)
#define pfiIXformDUpdateFunc(_ix, _func, _data) \
	    pfiIXformDUpdateFunc((pfiInputXformD *)_ix, _func, _data)
#define pfiGetIXformDUpdateFunc(_ix, _func, _data) \
	    pfiGetIXformDUpdateFunc((pfiInputXformD *)_ix, _func, _data)
#define pfiIXformDMotionFuncs(_ix, _start, _stop, _data) \
	    pfiIXformDMotionFuncs((pfiInputXformD *)_ix, _start, _stop, _data)
#define pfiGetIXformDMotionFuncs(_ix, _start, _stop, _data) \
	    pfiGetIXformDMotionFuncs((pfiInputXformD *)_ix, _start, _stop, _data)
#define pfiIXformDStartMotion(_xf, _startSpeed, _startAccel) \
		pfiIXformDStartMotion((pfiInputXformD *)_xf, _startSpeed, _startAccel);
#define pfiGetIXformDStartMotion(_xf, _startSpeed, _startAccel) \
		pfiGetIXformDStartMotion((pfiInputXformD *)_xf, _startSpeed, _startAccel);
#define pfiIXformDMotionLimits(_xf, _maxSpeed, _angularVel, _maxAccel) \
	    pfiIXformDMotionLimits((pfiInputXformD *)_xf, _maxSpeed, _angularVel, _maxAccel)
#define pfiUpdateIXformD(_ix) \
	    pfiUpdateIXformD((pfiInputXformD*)_ix)
#define pfiResetIXformDPosition(_ix) \
	    pfiResetIXformDPosition((pfiInputXformD*)_ix)
#define pfiIXformDMode(_ix, _mode, _val) \
	    pfiIXformDMode((pfiInputXformD*)_ix, _mode, _val)
#define pfiGetIXformDMode(_ix, _mode) \
	    pfiGetIXformDMode((pfiInputXformD*)_ix, _mode)
#define pfiIXformDMat(_ix, _mat) \
	    pfiIXformDMat((pfiInputXformD*)_ix, _mat)
#define pfiIXformDInputD(_ix, _xcoord) \
		pfiIXformDInputD((pfiInputXformD*)_ix, _in)
#define pfiGetIXformDInputD(_ix) \
		pfiGetIXformDInputD((pfiInputXformD*)_ix)
#define pfiIXformDInputCoordDPtr(_ix, _xcoord) \
		pfiIXformDInputCoordDPtr((pfiInputXformD*)_ix, _xcoord)
#define pfiGetIXformDInputCoordDPtr(_ix) \
		pfiGetIXformDInputCoordDPtr((pfiInputXformD*)_ix)
#define pfiIXformDMotionCoordD(_ix, _xcoord) \
		pfiIXformDMotionCoordD((pfiInputXformD*)_ix, _xcoord)
#define pfiGetIXformDMotionCoordD(_ix, _xcoord) \
		pfiGetIXformDMotionCoordD((pfiInputXformD*)_ix, _xcoord)
#define pfiIXformDResetCoord(_ix, _resetPos) \
		pfiIXformDResetCoord((pfiInputXformD *)_ix, _resetPos)
#define pfiGetIXformDResetCoord(_ix, _resetPos) \
		pfiGetIXformDResetCoord((pfiInputXformD *)_ix, _resetPos)
#define pfiGetIXformDMat(_ix, _mat) \
	    pfiGetIXformDMat((pfiInputXformD*)_ix, _mat)
#define pfiIXformDCoord(_ix, _coord) \
	    pfiIXformDCoord((pfiInputXformD*)_ix, _coord)
#define pfiGetIXformDCoord(_ix, _coord) \
	    pfiGetIXformDCoord((pfiInputXformD*)_ix, _coord)
#define pfiIXformDDBLimits(_xf, _dbLimits) \
		pfiIXformDDBLimits((pfiInputXformD*)_xf, _dbLimits)
#define pfiGetIXformDDBLimits(_xf, _dbLimits) \
		pfiGetIXformDDBLimits((pfiInputXformD*)_xf, _dbLimits)
#define pfiIXformDBSphere(_xf, _sphere) \
		pfiIXformDBSphere((pfiInputXformD*)_xf, _sphere)
#define pfiGetIXformDBSphere(_xf, _sphere) \
		pfiGetIXformDBSphere((pfiInputXformD*)_xf, _sphere)
#define pfiGetIXformDMotionLimits(_xf,_maxSpeed,_angularVel,_maxAccel) \
	    pfiGetIXformDMotionLimits((pfiInputXformD *)_xf,_maxSpeed,_angularVel,_maxAccel)

#endif /* __PFI_INPUTXFORM_C__ */

#ifndef _PFI_XFORMER_C_		/* can't def these for xformer.c */

#define pfiSelectXformerDModel(_xf,_which) \
		pfiSelectXformerDModel((pfiXformerD*)_xf,_which)
#define pfiGetXformerDCurModel(_xf) \
		pfiGetXformerDCurModel((pfiXformerD*)_xf)
#define pfiGetXformerDCurModelIndex( xf) \
		pfiGetXformerDCurModelIndex((pfiXformerD*) xf)
#define pfiStopXformerD(_xf) \
		pfiStopXformerD((pfiXformerD*)_xf)
#define pfiResetXformerD(_xf) \
		pfiResetXformerD((pfiXformerD*)_xf)
#define pfiResetXformerDPosition(_xf) \
		pfiResetXformerDPosition((pfiXformerD*)_xf)
#define pfiCenterXformerD(xf) \
		pfiCenterXformerD((pfiXformerD*) xf)
#define pfiXformerDAutoInput( xf, _chan,_mouse,_events) \
		pfiXformerDAutoInput((pfiXformerD*) xf, _chan,_mouse,_events)
#define pfiXformerDMat( xf,_mat) \
		pfiXformerDMat((pfiXformerD*) xf,_mat)
#define pfiGetXformerDMat( xf,_mat) \
		pfiGetXformerDMat((pfiXformerD*) xf,_mat)
#define pfiXformerDModelMat( xf,_mat) \
		pfiXformerDModelMat((pfiXformerD*) xf,_mat)
#define pfiGetXformerDModelMat( xf,_mat) \
		pfiGetXformerDModelMat((pfiXformerD*) xf,_mat)
#define pfiXformerDCoord( xf,_coord) \
		pfiXformerDCoord((pfiXformerD*) xf,_coord)
#define pfiGetXformerDCoord( xf,_coord) \
		pfiGetXformerDCoord((pfiXformerD*) xf,_coord)
#define pfiXformerDResetCoord(_xf, _coord) \
		pfiXformerDResetCoord((pfiXformerD*) _xf, _coord)
#define pfiGetXformerDResetCoord(_xf, _coord) \
		pfiGetXformerDResetCoord((pfiXformerD*) _xf, _coord)
#define pfiXformerDNode(xf, _node) \
		pfiXformerDNode((pfiXformerD*) xf, (pfNode*)_node)
#define pfiGetXformerDNode(xf) \
		pfiGetXformerDNode((pfiXformerD*)xf)
#define pfiXformerDAutoPosition(_xf, _chan, _dcs) \
		pfiXformerDAutoPosition((pfiXformerD*) _xf, _chan,  _dcs)
#define pfiGetXformerDAutoPosition(_xf, _chan, _dcs) \
		pfiGetXformerDAutoPosition((pfiXformerD*) _xf, _chan, _dcs)
#define pfiXformerDLimits( xf,_maxSpeed,_angularVel, _maxAccel, _dbLimits) \
		pfiXformerDLimits((pfiXformerD*) xf,_maxSpeed,_angularVel, _maxAccel, _dbLimits)
#define pfiGetXformerDLimits( xf, _maxSpeed, _angularVel, _maxAccel, _dbLimits) \
		pfiGetXformerDLimits((pfiXformerD*) xf, _maxSpeed, _angularVel, _maxAccel, _dbLimits)
#define pfiEnableXformerDCollision( xf) \
		pfiEnableXformerDCollision((pfiXformerD*) xf)
#define pfiDisableXformerDCollision( xf) \
		pfiDisableXformerDCollision((pfiXformerD*) xf)
#define pfiGetXformerDCollisionEnable( xf) \
		pfiGetXformerDCollisionEnable((pfiXformerD*) xf)
#define pfiXformerDCollision( xf, _mode, _val, _node) \
		pfiXformerDCollision((pfiXformerD*) xf, _mode, _val, (pfNode*)_node)
#define pfiGetXformerDCollisionStatus( xf) \
		pfiGetXformerDCollisionStatus((pfiXformerD*) xf)
#define pfiUpdateXformerD( xf) \
		pfiUpdateXformerD((pfiXformerD*) xf)
#define pfiCollideXformerD( xf) \
		pfiCollideXformerD((pfiXformerD*) xf)

#endif	/* _PFI_XFORMER_C_ */

#endif	/* defined(__STDC__) || defined(__cplusplus) */

#ifdef __cplusplus
}
#endif

#endif /* __PFUI_D_H__ */
