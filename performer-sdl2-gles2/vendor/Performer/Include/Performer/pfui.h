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
 * file: pfui.h
 * --------------
 *
 * $Revision: 1.53 $
 * $Date: 2002/07/24 01:55:45 $
 *
 * IRIS Performer user interface library header.
 */

#ifndef __PFUI_H__
#define __PFUI_H__

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
typedef struct _pfiCollide pfiCollide;
typedef struct _pfiPick pfiPick;
typedef struct _pfiInputCoord pfiInputCoord;
typedef struct _pfi2DInputCoord pfi2DInputCoord;
typedef struct _pfiMotionCoord pfiMotionCoord;
typedef struct _pfiInput pfiInput;
typedef struct _pfiInputXform pfiInputXform;
typedef struct _pfiInputXformTrackball pfiInputXformTrackball;
typedef struct _pfiInputXformTravel pfiInputXformTravel;
typedef struct _pfiInputXformDrive pfiInputXformDrive;
typedef struct _pfiInputXformFly pfiInputXformFly;
typedef struct _pfiInputXformSpheric pfiInputXformSpheric;

#else

class pfiCollide;
class pfiPick;
class pfiInputCoord;
class pfi2DInputCoord;
class pfiMotionCoord;
class pfiInput;
class pfiInputXform;
class pfiInputXformTrackball;
class pfiInputXformTravel;
class pfiInputXformDrive;
class pfiInputXformFly;
class pfiInputXformSpheric;
#endif /* !PF_CPLUSPLUS_API */

extern PFUIDLLEXPORT void pfiInit(void);

	/*------------------------ pfiMotionCoord ------------------------*/

extern PFUIDLLEXPORT pfType* pfiGetMotionCoordClassType(void);
extern PFUIDLLEXPORT pfiMotionCoord *pfiNewMotionCoord(void *arena);

	/*------------------------ pfiInputCoord ------------------------*/

extern PFUIDLLEXPORT pfType* pfiGetInputCoordClassType(void);
extern PFUIDLLEXPORT pfiInputCoord *pfiNewInputCoord(void *arena);
extern PFUIDLLEXPORT void pfiInputCoordVec(pfiInputCoord *ic, float *_vec);
extern PFUIDLLEXPORT void pfiGetInputCoordVec(pfiInputCoord *ic, float *_vec);

	/*------------------------ pfiInputXform ------------------------*/

typedef int (*pfiEventStreamHandlerType)(pfiInput *, pfuEventStream *,  void *);
typedef int (*pfiInputXformFuncType)(pfiInputXform *, void *);
typedef int (*pfiInputXformUpdateFuncType)(pfiInputXform *, pfiInputCoord *, void *);


/* callback return Values */
#define PFI_CB_CONT	0
#define PFI_CB_TERM	1
#define PFI_CB_ABORT	2

extern PFUIDLLEXPORT pfiInput *pfiNewInput(void *arena);
extern PFUIDLLEXPORT void pfiInputName(pfiInput *in, const char *_name);
extern PFUIDLLEXPORT const char *pfiIsIXGetName(pfiInput *in);
extern PFUIDLLEXPORT void pfiInputFocus(pfiInput *in, int _focus);
extern PFUIDLLEXPORT int pfiGetInputFocus(pfiInput *in);
extern PFUIDLLEXPORT void pfiInputEventMask(pfiInput *in, int _emask);
extern PFUIDLLEXPORT int pfiGetInputEventMask(pfiInput *in);
extern PFUIDLLEXPORT void pfiInputEventStreamCollector(pfiInput *in, pfiEventStreamHandlerType _func, void *data);
extern PFUIDLLEXPORT void pfiGetInputEventStreamCollector(pfiInput *in, pfiEventStreamHandlerType *_func, void **data);
extern PFUIDLLEXPORT void pfiInputEventStreamProcessor(pfiInput *in, pfiEventStreamHandlerType _func, void *data);
extern PFUIDLLEXPORT void pfiGetInputEventStreamProcessor(pfiInput *in, pfiEventStreamHandlerType *_func, void **data);
extern PFUIDLLEXPORT void pfiInputEventHandler(pfiInput *in, pfuEventHandlerFuncType _func, void *data);
extern PFUIDLLEXPORT void pfiGetInputEventHandler(pfiInput *in, pfuEventHandlerFuncType *_func, void **data);
extern PFUIDLLEXPORT void pfiResetInput(pfiInput *_in);
extern PFUIDLLEXPORT void pfiCollectInputEvents(pfiInput *_in);
extern PFUIDLLEXPORT void pfiProcessInputEvents(pfiInput *_in);

extern PFUIDLLEXPORT int pfiHaveFastMouseClick(pfuMouse *mouse, int button, float msecs);

extern PFUIDLLEXPORT pfiInputXform *pfiNewIXform(void *arena);
extern PFUIDLLEXPORT void pfiIXformFocus(pfiInputXform *in, int _focus);
extern PFUIDLLEXPORT int pfiIsIXformInMotion(pfiInputXform *_ix);
extern PFUIDLLEXPORT void pfiStopIXform(pfiInputXform *_ix);
extern PFUIDLLEXPORT void pfiResetIXform(pfiInputXform *_ix);
extern PFUIDLLEXPORT void pfiUpdateIXform(pfiInputXform *_ix);
extern PFUIDLLEXPORT void pfiIXformMode(pfiInputXform *_ix, int mode, int val);
extern PFUIDLLEXPORT int pfiGetIXformMode(pfiInputXform *_ix, int mode);
extern PFUIDLLEXPORT void pfiResetIXformPosition(pfiInputXform *_ix);
extern PFUIDLLEXPORT void pfiIXformMat(pfiInputXform *_ix, PFMATRIX _mat);
extern PFUIDLLEXPORT void pfiGetIXformMat(pfiInputXform *_ix, PFMATRIX _mat);
extern PFUIDLLEXPORT void pfiIXformInput(pfiInputXform *_ix, pfiInput *_in);
extern PFUIDLLEXPORT pfiInput* pfiGetIXformInput(pfiInputXform *_ix);
extern PFUIDLLEXPORT void pfiIXformInputCoordPtr(pfiInputXform *_ix, pfiInputCoord *_xcoord);
extern PFUIDLLEXPORT pfiInputCoord* pfiGetIXformInputCoordPtr(pfiInputXform *_ix);
extern PFUIDLLEXPORT void pfiIXformMotionCoord(pfiInputXform *_ix, pfiMotionCoord *_xcoord);
extern PFUIDLLEXPORT void pfiGetIXformMotionCoord(pfiInputXform *_ix, pfiMotionCoord *_xcoord);
extern PFUIDLLEXPORT void pfiIXformResetCoord(pfiInputXform *_ix, pfCoord *_resetPos);
extern PFUIDLLEXPORT void pfiGetIXformResetCoord(pfiInputXform *_ix, pfCoord *_resetPos);
extern PFUIDLLEXPORT void pfiIXformCoord(pfiInputXform *_ix, pfCoord *_coord);
extern PFUIDLLEXPORT void pfiGetIXformCoord(pfiInputXform *_ix, pfCoord *_coord);
extern PFUIDLLEXPORT void pfiIXformStartMotion(pfiInputXform *_xf, float _startSpeed, float _startAccel);
extern PFUIDLLEXPORT void pfiGetIXformStartMotion(pfiInputXform *_xf, float *_startSpeed, float *_startAccel);
extern PFUIDLLEXPORT void pfiIXformMotionLimits(pfiInputXform *_xf, float _maxSpeed, float _angularVel, float _maxAccel);
extern PFUIDLLEXPORT void pfiGetIXformMotionLimits(pfiInputXform *_xf, float *_maxSpeed, float *_angularVel, float *_maxAccel);
extern PFUIDLLEXPORT void pfiIXformDBLimits(pfiInputXform *_xf, pfBox *_dbLimits);
extern PFUIDLLEXPORT void pfiGetIXformDBLimits(pfiInputXform *_xf, pfBox *_dbLimits);
extern PFUIDLLEXPORT void pfiIXformBSphere(pfiInputXform *_xf, pfSphere *_sphere);
extern PFUIDLLEXPORT void pfiGetIXformBSphere(pfiInputXform *_xf, pfSphere *_sphere);
extern PFUIDLLEXPORT void pfiIXformUpdateFunc(pfiInputXform *_ix, pfiInputXformUpdateFuncType _func, void *_data);
extern PFUIDLLEXPORT void pfiGetIXformUpdateFunc(pfiInputXform *_ix, pfiInputXformUpdateFuncType *_func, void **_data);
extern PFUIDLLEXPORT void pfiIXformMotionFuncs(pfiInputXform *_ix, pfiInputXformFuncType _start, pfiInputXformFuncType _stop, void *_data);
extern PFUIDLLEXPORT void pfiGetIXformMotionFuncs(pfiInputXform *_ix, pfiInputXformFuncType *_start, pfiInputXformFuncType *_stop, void **_data);

/* General InputXform Modes to be accepted by all */
#define PFIX_MODE_MOTION	0
#define PFIX_MODE_MOTION_MOD	1 /* MOTION_MOD bitmask */
#define PFIX_MODE_ACCEL		2
#define PFIX_MODE_AUTO		3
#define PFIX_MODE_LIMIT_POS	4 /* PFIX_LIMIT_POS bitmask */
#define PFIX_MODE_LIMIT_SPEED	5 /* PFIX_LIMIT_SPEED bitmask */
#define PFIX_MODE_LIMIT_ACCEL	6 /* PFIX_LIMIT_ACCEL bitmask */

/* InputXform Mode values */
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

extern PFUIDLLEXPORT pfiInputXformTrackball *pfiNewIXformTrackball(void *_arena);
extern PFUIDLLEXPORT void pfiIXformTrackballMode(pfiInputXformTrackball *_tb, int _mode, int _val);
extern PFUIDLLEXPORT int  pfiGetIXformTrackballMode(pfiInputXformTrackball *_tb, int _mode);
extern PFUIDLLEXPORT pfiInputXformTrackball *pfiCreate2DIXformTrackball(void *arena);
extern PFUIDLLEXPORT int pfiUpdate2DIXformTrackball(pfiInputXform *_tb, pfiInputCoord *_icoord, void *data);

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

extern PFUIDLLEXPORT pfiInputXformSpheric *pfiNewIXSpheric(void *arena);
extern PFUIDLLEXPORT void pfiIXformSphericMode(pfiInputXformSpheric *spheric, int mode,
				 int val);
extern PFUIDLLEXPORT int pfiGetIXformSphericMode(pfiInputXformSpheric *spheric, int mode);
extern PFUIDLLEXPORT void pfiIXformSphericParameter(pfiInputXformSpheric *spheric,
				      int _param, float _val);
extern PFUIDLLEXPORT float pfiGetIXformSphericParameter(pfiInputXformSpheric *spheric,
					  int _param);
extern PFUIDLLEXPORT pfiInputXformSpheric *pfiCreate2DIXformSpheric(void *arnea);
extern PFUIDLLEXPORT int pfiUpdate2DIXformSpheric(pfiInputXform *spheric,
				    pfiInputCoord *_icoord, void *data);

void PFUIDLLEXPORT pfiIXformSphericSetWorld(pfiInputXformSpheric *spheric, int worldNumber,
			      int in_or_out);
void PFUIDLLEXPORT pfiIXformSphericReadPathFile(pfiInputXformSpheric *spheric,
				  char *filename);
void PFUIDLLEXPORT pfiIXformSphericPrintPathStuff(pfiInputXformSpheric *spheric);

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

extern PFUIDLLEXPORT pfType *pfiGetIXformTravelClassType(void);
extern PFUIDLLEXPORT pfType *pfiGetIXformDriveClassType(void);
extern PFUIDLLEXPORT pfType *pfiGetIXformFlyClassType(void);
extern PFUIDLLEXPORT pfType *pfiGetIXformSphericClassType(void);
extern PFUIDLLEXPORT pfType *pfiGetIXformTrackballClassType(void);


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

extern PFUIDLLEXPORT pfiInputXformDrive *pfiNewIXformDrive(void *arena);
extern PFUIDLLEXPORT void pfiIXformDriveMode(pfiInputXformDrive *_drive, int mode, int val);
extern PFUIDLLEXPORT int  pfiGetIXformDriveMode(pfiInputXformDrive *_drive, int mode);
extern PFUIDLLEXPORT void pfiIXformDriveHeight(pfiInputXformDrive* _drive, float height);
extern PFUIDLLEXPORT float pfiGetIXformDriveHeight(pfiInputXformDrive* _drive);
extern PFUIDLLEXPORT pfiInputXformDrive *pfiCreate2DIXformDrive(void *arena);
extern PFUIDLLEXPORT int pfiUpdate2DIXformDrive(pfiInputXform *_drive, pfiInputCoord *_icoord, void *data);

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

extern PFUIDLLEXPORT pfiInputXformFly *pfiNewIXFly(void *arena);
extern PFUIDLLEXPORT void pfiIXformFlyMode(pfiInputXformFly *_fly, int mode, int val);
extern PFUIDLLEXPORT int  pfiGetIXformFlyMode(pfiInputXformFly *_fly, int mode);
extern PFUIDLLEXPORT pfiInputXformFly *pfiCreate2DIXformFly(void *arnea);
extern PFUIDLLEXPORT int pfiUpdate2DIXformFly(pfiInputXform *_fly, pfiInputCoord *_icoord, void *data);

	/*------------------------ pfiCollide --------------------------*/

typedef int (*pfiCollideFuncType)(pfiCollide *, void *);

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

extern PFUIDLLEXPORT pfType *pfiGetCollideClassType(void);

extern PFUIDLLEXPORT pfiCollide * pfiNewCollide(void *_arena);
extern PFUIDLLEXPORT void pfiEnableCollide(pfiCollide *collide);
extern PFUIDLLEXPORT void pfiDisableCollide(pfiCollide *collide);
extern PFUIDLLEXPORT int pfiGetCollideEnable(pfiCollide *collide);
extern PFUIDLLEXPORT void pfiCollideMode(pfiCollide *collide, int _mode, int _val);
extern PFUIDLLEXPORT int pfiGetCollideMode(pfiCollide *collide, int _mode);
extern PFUIDLLEXPORT void pfiCollideStatus(pfiCollide *collide, int _status);
extern PFUIDLLEXPORT int pfiGetCollideStatus(pfiCollide *collide);
extern PFUIDLLEXPORT void pfiCollideDist(pfiCollide *collide, float _dist);
extern PFUIDLLEXPORT float pfiGetCollideDist(pfiCollide *collide);
extern PFUIDLLEXPORT void pfiCollideHeightAboveGrnd(pfiCollide *collide, float _dist);
extern PFUIDLLEXPORT float pfiGetCollideHeightAboveGrnd(pfiCollide *collide);
extern PFUIDLLEXPORT void pfiCollideGroundNode(pfiCollide *collide, pfNode* _ground);
extern PFUIDLLEXPORT pfNode * pfiGetCollideGroundNode(pfiCollide *collide);
extern PFUIDLLEXPORT void pfiCollideObjNode(pfiCollide *collide, pfNode* _db);
extern PFUIDLLEXPORT pfNode * pfiGetCollideObjNode(pfiCollide *collide);
extern PFUIDLLEXPORT void pfiCollideCurMotionParams(pfiCollide *collide, pfCoord* _pos, pfCoord* _prevPos, float _speed);
extern PFUIDLLEXPORT void pfiGetCollideCurMotionParams(pfiCollide *collide, pfCoord* _pos, pfCoord* _prevPos, float *_speed);
extern PFUIDLLEXPORT void pfiGetCollideMotionCoord(pfiCollide *collide, pfiMotionCoord* _xcoord);
extern PFUIDLLEXPORT void pfiCollideFunc(pfiCollide *collide, pfiCollideFuncType _func, void *_data);
extern PFUIDLLEXPORT void pfiGetCollisionFunc(pfiCollide *collide, pfiCollideFuncType *_func, void **_data);
extern PFUIDLLEXPORT int pfiUpdateCollide(pfiCollide *collide);

	/*---------------------------- pfiPick -----------------------------*/

/* Pick Modes */
#define PFIP_MODE_PICK		1 /* PFPK_M_NEAREST or PFPK_M_ALL */
#define PFIP_MODE_ISECT		2 /* PFTRAV_IS_* bitmasks */

typedef int (*pfiPickFuncType)(pfiPick *, void *);

extern PFUIDLLEXPORT pfType *pfiGetPickClassType(void);

extern PFUIDLLEXPORT pfiPick *	 pfiNewPick(void *arena);
extern PFUIDLLEXPORT void		 pfiPickMode(pfiPick *pick, int _mode, int _val);
extern PFUIDLLEXPORT int		 pfiGetPickMode(pfiPick *pick, int _mode);
extern PFUIDLLEXPORT void		 pfiPickHitFunc(pfiPick *pick, pfiPickFuncType _func, void *_data);
extern PFUIDLLEXPORT void		 pfiGetPicktHitFunc(pfiPick *pick, pfiPickFuncType *_func, void **_data);
extern PFUIDLLEXPORT void		 pfiAddPickChan(pfiPick *pick, pfChannel *_chan);
extern PFUIDLLEXPORT void		 pfiInsertPickChan(pfiPick *pick, int _index, pfChannel *_chan);
extern PFUIDLLEXPORT void		 pfiRemovePickChan(pfiPick *pick, pfChannel *_chan);
extern PFUIDLLEXPORT int		 pfiGetPickNumHits(pfiPick *pick);
extern PFUIDLLEXPORT pfNode *		 pfiGetPickNode(pfiPick *pick);
extern PFUIDLLEXPORT pfGeoSet *	 pfiGetPickGSet(pfiPick *pick);
extern PFUIDLLEXPORT void		 pfiSetupPickChans(pfiPick *pick);
extern PFUIDLLEXPORT int		 pfiDoPick(pfiPick *pick, int _x, int _y);
extern PFUIDLLEXPORT void		 pfiResetPick(pfiPick *pick);

	/*------------------------ pfiXformer --------------------------*/

#if !PF_CPLUSPLUS_API
typedef struct _pfiXformer 	pfiXformer;
#else
class pfiXformer;
#endif

/* XFormer Input Models */
#define PFIXF_INPUT_MOUSE_POLLED 200
#define PFIXF_INPUT_EVENTS	 210

extern PFUIDLLEXPORT pfType* pfiGetXformerClassType(void);
extern PFUIDLLEXPORT pfiXformer * pfiNewXformer(void* arena);
extern PFUIDLLEXPORT void pfiXformerModel(pfiXformer* xf, int _index, pfiInputXform* _model);
extern PFUIDLLEXPORT void pfiSelectXformerModel(pfiXformer* xf, int which);
extern PFUIDLLEXPORT pfiInputXform* pfiGetXformerCurModel(pfiXformer* xf);
extern PFUIDLLEXPORT int pfiGetXformerCurModelIndex(pfiXformer* xf);
extern PFUIDLLEXPORT int pfiRemoveXformerModel(pfiXformer* xf, int _index);
extern PFUIDLLEXPORT int pfiRemoveXformerModelIndex(pfiXformer* xf, pfiInputXform* _model);
extern PFUIDLLEXPORT void pfiStopXformer(pfiXformer* xf);
extern PFUIDLLEXPORT void pfiResetXformer(pfiXformer* xf);
extern PFUIDLLEXPORT void pfiResetXformerPosition(pfiXformer* xf);
extern PFUIDLLEXPORT void pfiCenterXformer(pfiXformer* xf);
extern PFUIDLLEXPORT void pfiXformerAutoInput(pfiXformer* xf, pfChannel* chan, pfuMouse* mouse, pfuEventStream* events);

extern PFUIDLLEXPORT void pfiXformerMat(pfiXformer* xf, PFMATRIX mat);
extern PFUIDLLEXPORT void pfiGetXformerMat(pfiXformer* xf, PFMATRIX mat);
extern PFUIDLLEXPORT void pfiXformerModelMat(pfiXformer* xf, PFMATRIX mat);
extern PFUIDLLEXPORT void pfiGetXformerModelMat(pfiXformer* xf, PFMATRIX mat);

extern PFUIDLLEXPORT void pfiXformerCoord(pfiXformer* xf, pfCoord *coord);
extern PFUIDLLEXPORT void pfiGetXformerCoord(pfiXformer* xf, pfCoord *coord);
extern PFUIDLLEXPORT void pfiXformerResetCoord(pfiXformer* xf, pfCoord *_resetPos);
extern PFUIDLLEXPORT void pfiGetXformerResetCoord(pfiXformer* xf, pfCoord *_resetPos);
extern PFUIDLLEXPORT void pfiXformerNode(pfiXformer* xf, pfNode *_node);
extern PFUIDLLEXPORT pfNode *pfiGetXformerNode(pfiXformer* xf);
extern PFUIDLLEXPORT void pfiXformerAutoPosition(pfiXformer* xf, pfChannel *_chan, pfDCS *_dcs);
extern PFUIDLLEXPORT void pfiGetXformerAutoPosition(pfiXformer* xf, pfChannel **_chan, pfDCS **_dcs);
extern PFUIDLLEXPORT void pfiXformerLimits(pfiXformer* xf, float maxSpeed, float angularVel, float maxAccel, pfBox* dbLimits);
extern PFUIDLLEXPORT void pfiGetXformerLimits(pfiXformer* xf, float *maxSpeed, float *angularVel, float *maxAccel, pfBox* dbLimits);
extern PFUIDLLEXPORT void pfiEnableXformerCollision(pfiXformer* xf);
extern PFUIDLLEXPORT void pfiDisableXformerCollision(pfiXformer* xf);
extern PFUIDLLEXPORT int pfiGetXformerCollisionEnable(pfiXformer* xf);
extern PFUIDLLEXPORT void pfiXformerCollision(pfiXformer* xf, int mode, float val, pfNode* node);
extern PFUIDLLEXPORT int pfiGetXformerCollisionStatus(pfiXformer* xf);
extern PFUIDLLEXPORT void pfiUpdateXformer(pfiXformer* xf);
extern PFUIDLLEXPORT int pfiCollideXformer(pfiXformer* xf);

/* Trackball-Drive-Fly Xformer (TDF) */


#if !PF_CPLUSPLUS_API
typedef struct _pfiTDFXformer 	pfiTDFXformer;
#else
class pfiTDFXformer;
#endif

/* TDF XFormer Input Models - indices for set/select of model for the
 * first three models
 */
#define PFITDF_TRACKBALL	    0
#define PFITDF_FLY		    1
#define PFITDF_DRIVE		    2
#define PFITDF_SPHERIC		    3

extern PFUIDLLEXPORT pfType* pfiGetTDFXformerClassType(void);
extern PFUIDLLEXPORT pfiTDFXformer * pfiNewTDFXformer(void* arena);
extern PFUIDLLEXPORT pfiXformer * pfiCreateTDFXformer( pfiInputXformTrackball *_tb, pfiInputXformDrive *_drive, pfiInputXformFly *_fly, void *arena);
extern PFUIDLLEXPORT void pfiTDFXformerFastClickTime(pfiTDFXformer* xf, float _time);
extern PFUIDLLEXPORT float pfiGetTDFXformerFastClickTime(pfiXformer* xf);
extern PFUIDLLEXPORT void pfiTDFXformerTrackball(pfiTDFXformer *_xf, pfiInputXformTrackball *_tb);
extern PFUIDLLEXPORT pfiInputXformTrackball *pfiGetTDFXformerTrackball(pfiTDFXformer *_xf);
extern PFUIDLLEXPORT void pfiTDFXformerDrive(pfiTDFXformer *_xf, pfiInputXformDrive *_tb);
extern PFUIDLLEXPORT pfiInputXformFly *pfiGetTDFXformerFly(pfiTDFXformer *_xf);
extern PFUIDLLEXPORT void pfiTDFXformerFly(pfiTDFXformer *_xf, pfiInputXformFly *_tb);
extern PFUIDLLEXPORT pfiInputXformDrive *pfiGetTDFXformerDrive(pfiTDFXformer *_xf);
extern PFUIDLLEXPORT void pfiTDFXformerSpheric(pfiTDFXformer *_xf, pfiInputXformSpheric *_tb);
extern PFUIDLLEXPORT pfiInputXformSpheric *pfiGetTDFXformerSpheric(pfiTDFXformer *_xf);

/* these routines implement the TDF mouse-based user-interface */
extern PFUIDLLEXPORT int pfiProcessTDFXformerMouseEvents(pfiInput *, pfuEventStream *,  void *data);
extern PFUIDLLEXPORT void pfiProcessTDFXformerMouse(pfiTDFXformer *xf, pfuMouse *mouse, pfChannel *inputChan);
extern PFUIDLLEXPORT void pfiProcessTDFTrackballMouse(pfiTDFXformer *xf, pfiInputXformTrackball *trackball, pfuMouse *mouse);
extern PFUIDLLEXPORT void pfiProcessTDFSphericMouse(pfiTDFXformer *xf, pfiInputXformSpheric *spheric, pfuMouse *mouse);
extern PFUIDLLEXPORT void pfiProcessTDFTravelMouse(pfiTDFXformer *xf, pfiInputXformTravel *tr, pfuMouse *mouse);

	/* ----------------- Macros for 1.2 Compatibility ----------------- */

#ifdef PF_COMPAT_1_2

typedef struct pfiXformer 		pfuXformer;

#define PFUXF_TRACKBALL			0
#define PFUXF_DRIVE			1
#define PFUXF_FLY			2

#define pfuNewXformer(_a)		pfiNewTDFXformer(_a)
#define pfuXformerMode(_xf, _m)		pfiSelectXformerModel(_xf, _m)
#define pfuGetXformerMode(_xf)		pfiGetXformerCurModelIndex(_xf)
#define pfuStopXformer(_xf)		pfiStopXformer(_xf)
#define pfuXformerAutoInput(_xf, _chan, _mouse, _events) \
		pfiXformerAutoInput(_xf, _chan, _mouse, _events)
#define pfuXformerMat(_xf, _mat)    pfiXformerMat(_xf, _mat)
#define pfuGetXformerMat(_xf, _mat) pfiGetXformerMat(_xf, _mat)
#define pfuXformerCoord(_xf, _coord)	pfiXformerCoord(_xf, _coord)
#define pfuGetXformerCoord(_xf, _coord)	pfiGetXformerCoord(_xf, _coord)
#define pfuXformerLimits(_xf, _maxSpeed, _angularVel, _maxAccel, _dbLimits) \
		pfiXformerLimits(_xf, _maxSpeed, _angularVel, _maxAccel, _dbLimits)
#define pfuGetXformerLimits(_xf, _maxSpeed, _angularVel, _maxAccel, _dbLimits) \
		pfiGetXformerLimits(_xf, _maxSpeed, _angularVel, _maxAccel, _dbLimits)
#define pfuXformerCollision(_xf, _mode, _val,_node) \
		pfiXformerCollision(_xf, _mode, _val,_node)
#define pfuGetXformerCollisionStatus(_xf) \
		pfiGetXformerCollisionStatus(_xf)
#define pfuUpdateXformer(_xf)		pfiUpdateXformer(_xf)
#define pfuCollideXformer(_xf)		pfiCollideXformer(_xf)

#endif /* PF_COMPAT_1_2 */

	/* ------------------- Macros for typecasting --------------------- */


#if defined(__STDC__) || defined(__cplusplus)

#ifndef __PFI_INPUTXFORM_C__

#define pfiInputName(_ix, _name) \
	    pfiInputName((pfiInput *)_ix, _name)
#define pfiGetInputName(_ix) \
	    pfiGetInputName((pfiInput *)_ix)
#define pfiIsInputInMotion(_ix) \
	    pfiIsInputInMotion(_ix)
#define pfiInputFocus(_ix, _focus) \
	    pfiInputFocus((pfiInput *)_ix, _focus)
#define pfiGetInputFocus(_ix) \
	    pfiGetInputFocus((pfiInput *)_ix)
#define pfiInputEventMask(_ix, _emask) \
	    pfiInputEventMask((pfiInput *)_ix, _emask)
#define pfiGetInputEventMask(_ix) \
	    pfiGetInputEventMask((pfiInput *)_ix)
#define pfiInputEventStreamCollector(_ix, _func) \
	    pfiInputEventStreamCollector(_ix, _func)
#define pfiGetInputEventStreamCollector(_ix, _func, _data) \
	    pfiGetInputEventStreamCollector((pfiInput *)_ix, _func, _data)
#define pfiInputEventStreamProcessor(_ix, _func) \
	    pfiInputEventStreamProcessor(_ix, _func)
#define pfiGetInputEventStreamProcessor(_ix, _func, _data) \
	    pfiGetInputEventStreamProcessor((pfiInput *)_ix, _func, _data)
#define pfiInputEventHandler(_ix, _func) \
	    pfiInputEventHandler(_ix, _func)
#define pfiGetInputEventHandler(_ix, _func, _data) \
	    pfiGetInputEventHandler((pfiInput *)_ix, _func, _data)
#define pfiResetInput(_ix) \
	    pfiResetInput((pfiInput *)_ix)
#define pfiCollectInputEvents(_ix) \
	    pfiCollectInputEvents((pfiInput *)_ix)
#define pfiProcessInputEvents(_ix) \
	    pfiProcessInputEvents((pfiInput *)_ix)

#define pfiInputCoordVec(_ic, _vec) \
	    pfiInputCoordVec((pfiInputCoord*)_ic, _vec)
#define pfiGetInputCoordVec(_ic, _vec) \
	    pfiGetInputCoordVec((pfiInputCoord*)_ic, _vec)

#define pfiIXformFocus(_ix, _focus) \
	    pfiIXformFocus((pfiInputXform *)_ix, _focus)
#define pfiStopIXform(_ix) \
	    pfiStopIXform((pfiInputXform *)_ix)
#define pfiResetIXform(_ix) \
	    pfiResetIXform((pfiInputXform *)_ix)
#define pfiIXformUpdateFunc(_ix, _func, _data) \
	    pfiIXformUpdateFunc((pfiInputXform *)_ix, _func, _data)
#define pfiGetIXformUpdateFunc(_ix, _func, _data) \
	    pfiGetIXformUpdateFunc((pfiInputXform *)_ix, _func, _data)
#define pfiIXformMotionFuncs(_ix, _start, _stop, _data) \
	    pfiIXformMotionFuncs((pfiInputXform *)_ix, _start, _stop, _data)
#define pfiGetIXformMotionFuncs(_ix, _start, _stop, _data) \
	    pfiGetIXformMotionFuncs((pfiInputXform *)_ix, _start, _stop, _data)
#define pfiIXformStartMotion(_xf, _startSpeed, _startAccel) \
		pfiIXformStartMotion((pfiInputXform *)_xf, _startSpeed, _startAccel);
#define pfiGetIXformStartMotion(_xf, _startSpeed, _startAccel) \
		pfiGetIXformStartMotion((pfiInputXform *)_xf, _startSpeed, _startAccel);
#define pfiIXformMotionLimits(_xf, _maxSpeed, _angularVel, _maxAccel) \
	    pfiIXformMotionLimits((pfiInputXform *)_xf, _maxSpeed, _angularVel, _maxAccel)
#define pfiUpdateIXform(_ix) \
	    pfiUpdateIXform((pfiInputXform*)_ix)
#define pfiResetIXformPosition(_ix) \
	    pfiResetIXformPosition((pfiInputXform*)_ix)
#define pfiIXformMode(_ix, _mode, _val) \
	    pfiIXformMode((pfiInputXform*)_ix, _mode, _val)
#define pfiGetIXformMode(_ix, _mode) \
	    pfiGetIXformMode((pfiInputXform*)_ix, _mode)
#define pfiIXformMat(_ix, _mat) \
	    pfiIXformMat((pfiInputXform*)_ix, _mat)
#define pfiIXformInput(_ix, _xcoord) \
		pfiIXformInput((pfiInputXform*)_ix, _in)
#define pfiGetIXformInput(_ix) \
		pfiGetIXformInput((pfiInputXform*)_ix)
#define pfiIXformInputCoordPtr(_ix, _xcoord) \
		pfiIXformInputCoordPtr((pfiInputXform*)_ix, _xcoord)
#define pfiGetIXformInputCoordPtr(_ix) \
		pfiGetIXformInputCoordPtr((pfiInputXform*)_ix)
#define pfiIXformMotionCoord(_ix, _xcoord) \
		pfiIXformMotionCoord((pfiInputXform*)_ix, _xcoord)
#define pfiGetIXformMotionCoord(_ix, _xcoord) \
		pfiGetIXformMotionCoord((pfiInputXform*)_ix, _xcoord)
#define pfiIXformResetCoord(_ix, _resetPos) \
		pfiIXformResetCoord((pfiInputXform *)_ix, _resetPos)
#define pfiGetIXformResetCoord(_ix, _resetPos) \
		pfiGetIXformResetCoord((pfiInputXform *)_ix, _resetPos)
#define pfiGetIXformMat(_ix, _mat) \
	    pfiGetIXformMat((pfiInputXform*)_ix, _mat)
#define pfiIXformCoord(_ix, _coord) \
	    pfiIXformCoord((pfiInputXform*)_ix, _coord)
#define pfiGetIXformCoord(_ix, _coord) \
	    pfiGetIXformCoord((pfiInputXform*)_ix, _coord)
#define pfiIXformDBLimits(_xf, _dbLimits) \
		pfiIXformDBLimits((pfiInputXform*)_xf, _dbLimits)
#define pfiGetIXformDBLimits(_xf, _dbLimits) \
		pfiGetIXformDBLimits((pfiInputXform*)_xf, _dbLimits)
#define pfiIXformBSphere(_xf, _sphere) \
		pfiIXformBSphere((pfiInputXform*)_xf, _sphere)
#define pfiGetIXformBSphere(_xf, _sphere) \
		pfiGetIXformBSphere((pfiInputXform*)_xf, _sphere)
#define pfiGetIXformMotionLimits(_xf,_maxSpeed,_angularVel,_maxAccel) \
	    pfiGetIXformMotionLimits((pfiInputXform *)_xf,_maxSpeed,_angularVel,_maxAccel)

#endif /* __PFI_INPUTXFORM_C__ */

#ifndef _PFI_XFORMER_C_		/* can't def these for xformer.c */

#define pfiSelectXformerModel(_xf,_which) \
		pfiSelectXformerModel((pfiXformer*)_xf,_which)
#define pfiGetXformerCurModel(_xf) \
		pfiGetXformerCurModel((pfiXformer*)_xf)
#define pfiGetXformerCurModelIndex( xf) \
		pfiGetXformerCurModelIndex((pfiXformer*) xf)
#define pfiStopXformer(_xf) \
		pfiStopXformer((pfiXformer*)_xf)
#define pfiResetXformer(_xf) \
		pfiResetXformer((pfiXformer*)_xf)
#define pfiResetXformerPosition(_xf) \
		pfiResetXformerPosition((pfiXformer*)_xf)
#define pfiCenterXformer(xf) \
		pfiCenterXformer((pfiXformer*) xf)
#define pfiXformerAutoInput( xf, _chan,_mouse,_events) \
		pfiXformerAutoInput((pfiXformer*) xf, _chan,_mouse,_events)
#define pfiXformerMat( xf,_mat) \
		pfiXformerMat((pfiXformer*) xf,_mat)
#define pfiGetXformerMat( xf,_mat) \
		pfiGetXformerMat((pfiXformer*) xf,_mat)
#define pfiXformerModelMat( xf,_mat) \
		pfiXformerModelMat((pfiXformer*) xf,_mat)
#define pfiGetXformerModelMat( xf,_mat) \
		pfiGetXformerModelMat((pfiXformer*) xf,_mat)
#define pfiXformerCoord( xf,_coord) \
		pfiXformerCoord((pfiXformer*) xf,_coord)
#define pfiGetXformerCoord( xf,_coord) \
		pfiGetXformerCoord((pfiXformer*) xf,_coord)
#define pfiXformerResetCoord(_xf, _coord) \
		pfiXformerResetCoord((pfiXformer*) _xf, _coord)
#define pfiGetXformerResetCoord(_xf, _coord) \
		pfiGetXformerResetCoord((pfiXformer*) _xf, _coord)
#define pfiXformerNode(xf, _node) \
		pfiXformerNode((pfiXformer*) xf, (pfNode*)_node)
#define pfiGetXformerNode(xf) \
		pfiGetXformerNode((pfiXformer*)xf)
#define pfiXformerAutoPosition(_xf, _chan, _dcs) \
		pfiXformerAutoPosition((pfiXformer*) _xf, _chan,  _dcs)
#define pfiGetXformerAutoPosition(_xf, _chan, _dcs) \
		pfiGetXformerAutoPosition((pfiXformer*) _xf, _chan, _dcs)
#define pfiXformerLimits( xf,_maxSpeed,_angularVel, _maxAccel, _dbLimits) \
		pfiXformerLimits((pfiXformer*) xf,_maxSpeed,_angularVel, _maxAccel, _dbLimits)
#define pfiGetXformerLimits( xf, _maxSpeed, _angularVel, _maxAccel, _dbLimits) \
		pfiGetXformerLimits((pfiXformer*) xf, _maxSpeed, _angularVel, _maxAccel, _dbLimits)
#define pfiEnableXformerCollision( xf) \
		pfiEnableXformerCollision((pfiXformer*) xf)
#define pfiDisableXformerCollision( xf) \
		pfiDisableXformerCollision((pfiXformer*) xf)
#define pfiGetXformerCollisionEnable( xf) \
		pfiGetXformerCollisionEnable((pfiXformer*) xf)
#define pfiXformerCollision( xf, _mode, _val, _node) \
		pfiXformerCollision((pfiXformer*) xf, _mode, _val, (pfNode*)_node)
#define pfiGetXformerCollisionStatus( xf) \
		pfiGetXformerCollisionStatus((pfiXformer*) xf)
#define pfiUpdateXformer( xf) \
		pfiUpdateXformer((pfiXformer*) xf)
#define pfiCollideXformer( xf) \
		pfiCollideXformer((pfiXformer*) xf)

#endif	/* _PFI_XFORMER_C_ */

#endif	/* defined(__STDC__) || defined(__cplusplus) */

#ifdef __cplusplus
}
#endif

#endif /* __PFUI_H__ */
