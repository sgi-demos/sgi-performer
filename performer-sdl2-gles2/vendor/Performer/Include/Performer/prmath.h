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
 * prmath.h	Include file for Performer math functions.
 *
 * $Revision: 1.90 $
 * $Date: 2002/12/08 23:02:33 $
 *
 */

#ifndef __PRMATH_H__
#define __PRMATH_H__
#include <math.h>   /* Get all math routines */

/* Default enables C++ API for C++ compiles */
#ifndef PF_CPLUSPLUS_API
#ifdef __cplusplus
#define PF_CPLUSPLUS_API 1 /* enable C++ API, types */
#else
#define PF_CPLUSPLUS_API 0 /* do not enable C++ API, types */
#endif
#endif

/* Default only enables C API if not using C++ API */
#ifndef PF_C_API
#define PF_C_API !PF_CPLUSPLUS_API 
#endif

#if PF_CPLUSPLUS_API


#ifndef PFINTERNAL
#define PFINTERNAL      public 
#endif

#include <Performer/pfDLL.h>

class pfType;

/* public array structs */
struct  pfVec2;
struct  pfVec3;
struct  pfVec4;
struct  pfMatrix;
struct  pfQuat;
struct  pfVec2d;
struct  pfVec3d;
struct  pfVec4d;
struct  pfMatrix4d;
struct  pfQuatd;
#define PFVEC2 pfVec2&
#define PFVEC3 pfVec3&
#define PFVEC4 pfVec4&
#define PFQUAT pfQuat&
#define PFMATRIX pfMatrix&
#define PFVEC2D pfVec2d&
#define PFVEC3D pfVec3d&
#define PFVEC4D pfVec4d&
#define PFQUATD pfQuatd&
#define PFMATRIX4D pfMatrix4d&

/* public structs */
struct  pfCoord;
struct  pfCoordd;
struct 	pfSeg;
struct 	pfSegd;
struct 	pfSegSet;
struct 	pfSegSetd;
struct 	pfPlane;
struct 	pfSphere;
struct 	pfSpheredd;
struct 	pfSpheredf;
struct 	pfCylinder;
struct 	pfCylinderd;
struct 	pfBox;

/* opaque classes */
class	pfFrustum;
class	pfPolytope;
class	pfMatStack;
class	pfHit;

#else

typedef struct _pfType		pfType;

/* typedefed arrays */
typedef float   pfVec2[2];
typedef float   pfVec3[3];
typedef float   pfVec4[4];
typedef float   pfMatrix[4][4];
typedef pfVec4  pfQuat;
typedef double   pfVec2d[2];
typedef double   pfVec3d[3];
typedef double   pfVec4d[4];
typedef double   pfMatrix4d[4][4];
typedef pfVec4d  pfQuatd;

#define PFVEC2 pfVec2
#define PFVEC3 pfVec3
#define PFVEC4 pfVec4
#define PFQUAT pfQuat
#define PFMATRIX pfMatrix
#define PFVEC2D pfVec2d
#define PFVEC3D pfVec3d
#define PFVEC4D pfVec4d
#define PFQUATD pfQuatd
#define PFMATRIX4D pfMatrix4d

/* exposed structs */
typedef struct _pfSeg			pfSeg;
typedef struct _pfSegd			pfSegd;
typedef struct _pfSegSet		pfSegSet;
typedef struct _pfSegSetd		pfSegSetd;
typedef struct _pfCoord			pfCoord;
typedef struct _pfCoordd		pfCoordd;
typedef struct _pfPlane			pfPlane;
typedef struct _pfSphere		pfSphere;
typedef struct _pfSpheredd		pfSpheredd;
typedef struct _pfSpheredf		pfSpheredf;
typedef struct _pfCylinder		pfCylinder;
typedef struct _pfCylinderd		pfCylinderd;
typedef struct _pfBox			pfBox;

/* opaque classes */
typedef struct _pfFrustum     pfFrustum;
typedef struct _pfPolytope    pfPolytope;
typedef struct _pfMatStack    pfMatStack;
typedef struct _pfHit         pfHit;

#endif /* PF_CPLUSPLUS_API */

#ifdef __cplusplus
extern "C" {
#endif

extern void	pfFPConfig(int which, float val);
extern float	pfGetFPConfig(int which);

extern void	pfDPConfig(int which, double val);
extern double	pfGetDPConfig(int which);

/* extern void 	pfSinCos(float _arg, float* _s, float* _c); */ 
/* extern float 	pfTan(float _arg); */
/* Moved down */

/* extern float 	pfArcTan2(float _y, float _x); */ 
/* extern float 	pfArcSin(float _arg); */
/* extern float 	pfArcCos(float _arg); */
/* extern float 	pfSqrt(float _arg); */	                 

/*
 * ---------------------- C API --------------------------------
 */

#if PF_C_API


#ifdef PF_C_API
/*---------------- pfVec2 CAPI ------------------------------*/

extern DLLEXPORT void                 pfSetVec2(PFVEC2 _vec2, float _x, float _y);
extern DLLEXPORT void                 pfCopyVec2(PFVEC2 _vec2, const PFVEC2 _v);
extern DLLEXPORT int                  pfEqualVec2(const PFVEC2 _vec2, const PFVEC2 _v);
extern DLLEXPORT int                  pfAlmostEqualVec2(const PFVEC2 _vec2, const PFVEC2 _v, float _tol);
extern DLLEXPORT void                 pfNegateVec2(PFVEC2 _vec2, const PFVEC2 _v);
extern DLLEXPORT float                pfDotVec2(const PFVEC2 _vec2, const PFVEC2 _v);
extern DLLEXPORT void                 pfAddVec2(PFVEC2 _vec2, const PFVEC2 _v1, const PFVEC2 _v2);
extern DLLEXPORT void                 pfSubVec2(PFVEC2 _vec2, const PFVEC2 _v1, const PFVEC2 _v2);
extern DLLEXPORT void                 pfScaleVec2(PFVEC2 _vec2, float _s, const PFVEC2 _v);
extern DLLEXPORT void                 pfAddScaledVec2(PFVEC2 _vec2, const PFVEC2 _v1, float _s, const PFVEC2 _v2);
extern DLLEXPORT void                 pfCombineVec2(PFVEC2 _vec2, float _a, const PFVEC2 _v1, float _b, const PFVEC2 _v2);
extern DLLEXPORT float                pfSqrDistancePt2(const PFVEC2 _vec2, const PFVEC2 _v);
extern DLLEXPORT float                pfNormalizeVec2(PFVEC2 _vec2);
extern DLLEXPORT float                pfLengthVec2(const PFVEC2 _vec2);
extern DLLEXPORT float                pfDistancePt2(const PFVEC2 _vec2, const PFVEC2 _v);

#endif /* !PF_C_API */

#ifdef PF_C_API
/*---------------- pfVec3 CAPI ------------------------------*/

extern DLLEXPORT void                 pfSetVec3(PFVEC3 _vec3, float _x, float _y, float _z);
extern DLLEXPORT void                 pfCopyVec3(PFVEC3 _vec3, const PFVEC3 _v);
extern DLLEXPORT int                  pfEqualVec3(const PFVEC3 _vec3, const PFVEC3 _v);
extern DLLEXPORT int                  pfAlmostEqualVec3(const PFVEC3 _vec3, const PFVEC3 _v, float _tol);
extern DLLEXPORT void                 pfNegateVec3(PFVEC3 _vec3, const PFVEC3 _v);
extern DLLEXPORT float                pfDotVec3(const PFVEC3 _vec3, const PFVEC3 _v);
extern DLLEXPORT void                 pfAddVec3(PFVEC3 _vec3, const PFVEC3 _v1, const PFVEC3 _v2);
extern DLLEXPORT void                 pfSubVec3(PFVEC3 _vec3, const PFVEC3 _v1, const PFVEC3 _v2);
extern DLLEXPORT void                 pfScaleVec3(PFVEC3 _vec3, float _s, const PFVEC3 _v);
extern DLLEXPORT void                 pfAddScaledVec3(PFVEC3 _vec3, const PFVEC3 _v1, float _s, const PFVEC3 _v2);
extern DLLEXPORT void                 pfCombineVec3(PFVEC3 _vec3, float _a, const PFVEC3 _v1, float _b, const PFVEC3 _v2);
extern DLLEXPORT float                pfSqrDistancePt3(const PFVEC3 _vec3, const PFVEC3 _v);
extern DLLEXPORT float                pfNormalizeVec3(PFVEC3 _vec3);
extern DLLEXPORT float                pfLengthVec3(const PFVEC3 _vec3);
extern DLLEXPORT float                pfDistancePt3(const PFVEC3 _vec3, const PFVEC3 _v);
extern DLLEXPORT void                 pfCrossVec3(PFVEC3 _vec3, const PFVEC3 _v1, const PFVEC3 _v2);
extern DLLEXPORT void                 pfXformVec3(PFVEC3 _vec3, const PFVEC3 _v, const PFMATRIX _m);
extern DLLEXPORT void                 pfXformPt3(PFVEC3 _vec3, const PFVEC3 _v, const PFMATRIX _m);
extern DLLEXPORT void                 pfFullXformPt3(PFVEC3 _vec3, const PFVEC3 _v, const PFMATRIX _m);
extern DLLEXPORT float                pfFullXformPt3w(PFVEC3 _vec3, const PFVEC3 _v, const PFMATRIX _m);

#endif /* !PF_C_API */

#ifdef PF_C_API
/*---------------- pfVec4 CAPI ------------------------------*/

extern DLLEXPORT void                 pfSetVec4(PFVEC4 _vec4, float _x, float _y, float _z, float _w);
extern DLLEXPORT void                 pfCopyVec4(PFVEC4 _vec4, const PFVEC4 _v);
extern DLLEXPORT int                  pfEqualVec4(const PFVEC4 _vec4, const PFVEC4 _v);
extern DLLEXPORT int                  pfAlmostEqualVec4(const PFVEC4 _vec4, const PFVEC4 _v, float _tol);
extern DLLEXPORT void                 pfNegateVec4(PFVEC4 _vec4, const PFVEC4 _v);
extern DLLEXPORT float                pfDotVec4(const PFVEC4 _vec4, const PFVEC4 _v);
extern DLLEXPORT void                 pfAddVec4(PFVEC4 _vec4, const PFVEC4 _v1, const PFVEC4 _v2);
extern DLLEXPORT void                 pfSubVec4(PFVEC4 _vec4, const PFVEC4 _v1, const PFVEC4 _v2);
extern DLLEXPORT void                 pfScaleVec4(PFVEC4 _vec4, float _s, const PFVEC4 _v);
extern DLLEXPORT void                 pfAddScaledVec4(PFVEC4 _vec4, const PFVEC4 _v1, float _s, const PFVEC4 _v2);
extern DLLEXPORT void                 pfCombineVec4(PFVEC4 _vec4, float _a, const PFVEC4 _v1, float _b, const PFVEC4 _v2);
extern DLLEXPORT float                pfSqrDistancePt4(const PFVEC4 _vec4, const PFVEC4 _v);
extern DLLEXPORT float                pfNormalizeVec4(PFVEC4 _vec4);
extern DLLEXPORT float                pfLengthVec4(const PFVEC4 _vec4);
extern DLLEXPORT float                pfDistancePt4(const PFVEC4 _vec4, const PFVEC4 _v);
extern DLLEXPORT void                 pfXformVec4(PFVEC4 _vec4, const PFVEC4 v, const PFMATRIX m);

#endif /* !PF_C_API */

#ifdef PF_C_API
/*---------------- pfMatrix CAPI ------------------------------*/

#if 0
#endif
extern DLLEXPORT void                 pfSetMat(PFMATRIX _mat, float *m);
extern DLLEXPORT int                  pfGetMatType(const PFMATRIX _mat);
extern DLLEXPORT void                 pfSetMatRowVec3(PFMATRIX _mat, int _r, const PFVEC3 _v);
extern DLLEXPORT void                 pfSetMatRow(PFMATRIX _mat, int _r, float _x, float _y, float _z, float _w);
extern DLLEXPORT void                 pfGetMatRowVec3(const PFMATRIX _mat, int _r, PFVEC3 _dst);
extern DLLEXPORT void                 pfGetMatRow(const PFMATRIX _mat, int _r, float *_x, float *_y, float *_z, float *_w);
extern DLLEXPORT void                 pfSetMatColVec3(PFMATRIX _mat, int _c, const PFVEC3 _v);
extern DLLEXPORT void                 pfSetMatCol(PFMATRIX _mat, int _c, float _x, float _y, float _z, float _w);
extern DLLEXPORT void                 pfGetMatColVec3(const PFMATRIX _mat, int _c, PFVEC3 _dst);
extern DLLEXPORT void                 pfGetMatCol(const PFMATRIX _mat, int _c, float *_x, float *_y, float *_z, float *_w);
extern DLLEXPORT void                 pfGetOrthoMatCoord(const PFMATRIX _mat, pfCoord* _dst);
extern DLLEXPORT void                 pfMakeIdentMat(PFMATRIX _mat);
extern DLLEXPORT void                 pfMakeEulerMat(PFMATRIX _mat, float _hdeg, float _pdeg, float _rdeg);
extern DLLEXPORT void                 pfMakeRotMat(PFMATRIX _mat, float _degrees, float _x, float _y, float _z);
extern DLLEXPORT void                 pfMakeTransMat(PFMATRIX _mat, float _x, float _y, float _z);
extern DLLEXPORT void                 pfMakeScaleMat(PFMATRIX _mat, float _x, float _y, float _z);
extern DLLEXPORT void                 pfMakeVecRotVecMat(PFMATRIX _mat, const PFVEC3 _v1, const PFVEC3 _v2);
extern DLLEXPORT void                 pfMakeCoordMat(PFMATRIX _mat, const pfCoord* _c);
extern DLLEXPORT void                 pfGetOrthoMatQuat(const PFMATRIX _mat, PFQUAT dst);
extern DLLEXPORT void                 pfMakeQuatMat(PFMATRIX _mat, const PFQUAT q);
extern DLLEXPORT void                 pfCopyMat(PFMATRIX _mat, const PFMATRIX _v);
extern DLLEXPORT int                  pfEqualMat(const PFMATRIX _mat, const PFMATRIX _m);
extern DLLEXPORT int                  pfAlmostEqualMat(const PFMATRIX _mat, const PFMATRIX _m2, float _tol);
extern DLLEXPORT void                 pfTransposeMat(PFMATRIX _mat, PFMATRIX _m);
extern DLLEXPORT void                 pfMultMat(PFMATRIX _mat, const PFMATRIX _m1, const PFMATRIX m2);
extern DLLEXPORT void                 pfAddMat(PFMATRIX _mat, const PFMATRIX _m1, const PFMATRIX m2);
extern DLLEXPORT void                 pfSubMat(PFMATRIX _mat, const PFMATRIX _m1, const PFMATRIX m2);
/* pfScaleMat() multiplies full 4X4 by scalar, not a 3D scale xform */
extern DLLEXPORT void                 pfScaleMat(PFMATRIX _mat, float _s, const PFMATRIX _m);
extern DLLEXPORT void                 pfPostMultMat(PFMATRIX _mat, const PFMATRIX _m);
extern DLLEXPORT void                 pfPreMultMat(PFMATRIX _mat, const PFMATRIX _m);
extern DLLEXPORT int                  pfInvertFullMat(PFMATRIX _mat, PFMATRIX _m);
extern DLLEXPORT void                 pfInvertAffMat(PFMATRIX _mat, const PFMATRIX _m);
extern DLLEXPORT void                 pfInvertOrthoMat(PFMATRIX _mat, const PFMATRIX _m);
extern DLLEXPORT void                 pfInvertOrthoNMat(PFMATRIX _mat, PFMATRIX _m);
extern DLLEXPORT void                 pfInvertIdentMat(PFMATRIX _mat, const PFMATRIX _m);
extern DLLEXPORT void                 pfPreTransMat(PFMATRIX _mat, float _x, float _y, float _z, PFMATRIX _m);
extern DLLEXPORT void                 pfPostTransMat(PFMATRIX _mat, const PFMATRIX _m, float _x, float _y, float _z);
extern DLLEXPORT void                 pfPreRotMat(PFMATRIX _mat, float _degrees, float _x, float _y, float _z, PFMATRIX _m);
extern DLLEXPORT void                 pfPostRotMat(PFMATRIX _mat, const PFMATRIX _m, float _degrees, float _x, float _y, float _z);
extern DLLEXPORT void                 pfPreScaleMat(PFMATRIX _mat, float _xs, float _ys, float _zs, PFMATRIX _m);
extern DLLEXPORT void                 pfPostScaleMat(PFMATRIX _mat, const PFMATRIX _m, float _xs, float _ys, float _zs);

#endif /* !PF_C_API */

#if !PF_CPLUSPLUS_API /* Also in C++ header  EXPORT to CAPI */
/*
 * pfQuat derives from pfVec4.
 * pfVec4 routines can also operate on pfQuats.
 */
#endif /* !PF_CPLUSPLUS_API */

#ifdef PF_C_API
/*---------------- pfQuat CAPI ------------------------------*/

extern DLLEXPORT void                 pfGetQuatRot(const PFQUAT _quat, float *_angle, float *_x, float *_y, float *_z);
extern DLLEXPORT void                 pfMakeRotQuat(PFQUAT _quat, float _angle, float _x, float _y, float _z);
extern DLLEXPORT void                 pfRotQuat(PFQUAT _quat, PFMATRIX m);
extern DLLEXPORT void                 pfMakeMatRotQuat(PFQUAT _quat, const PFMATRIX m);
extern DLLEXPORT void                 pfMakeVecRotVecQuat(PFQUAT _quat, const PFVEC3 rotateFrom, const pfVec3 rotateTo);
extern DLLEXPORT void                 pfConjQuat(PFQUAT _quat, const PFQUAT _v);
extern DLLEXPORT float                pfLengthQuat(const PFQUAT _quat);
extern DLLEXPORT void                 pfMultQuat(PFQUAT _quat, const PFQUAT q1, const PFQUAT q2);
extern DLLEXPORT void                 pfDivQuat(PFQUAT _quat, const PFQUAT q1, const PFQUAT q2);
extern DLLEXPORT void                 pfInvertQuat(PFQUAT _quat, const PFQUAT q1);
extern DLLEXPORT void                 pfExpQuat(PFQUAT _quat, const PFQUAT _q);
extern DLLEXPORT void                 pfLogQuat(PFQUAT _quat, const PFQUAT _q);
extern DLLEXPORT void                 pfSlerpQuat(PFQUAT _quat, float _t, const PFQUAT _q1, const PFQUAT _q2);
extern DLLEXPORT void                 pfSquadQuat(PFQUAT _quat, float _t, const PFQUAT _q1, const PFQUAT _q2, const PFQUAT _a, const PFQUAT _b);
extern DLLEXPORT void                 pfQuatMeanTangent(PFQUAT _quat, const PFQUAT _q1, const PFQUAT _q2, const PFQUAT _q3);

#endif /* !PF_C_API */

#ifdef PF_C_API
/*---------------- pfVec2d CAPI ------------------------------*/

extern DLLEXPORT void                 pfSetVec2d(PFVEC2D _vec2d, double _x, double _y);
extern DLLEXPORT void                 pfCopyVec2d(PFVEC2D _vec2d, const PFVEC2D _v);
extern DLLEXPORT int                  pfEqualVec2d(const PFVEC2D _vec2d, const PFVEC2D _v);
extern DLLEXPORT int                  pfAlmostEqualVec2d(const PFVEC2D _vec2d, const PFVEC2D _v, double _tol);
extern DLLEXPORT void                 pfNegateVec2d(PFVEC2D _vec2d, const PFVEC2D _v);
extern DLLEXPORT double               pfDotVec2d(const PFVEC2D _vec2d, const PFVEC2D _v);
extern DLLEXPORT void                 pfAddVec2d(PFVEC2D _vec2d, const PFVEC2D _v1, const PFVEC2D _v2);
extern DLLEXPORT void                 pfSubVec2d(PFVEC2D _vec2d, const PFVEC2D _v1, const PFVEC2D _v2);
extern DLLEXPORT void                 pfScaleVec2d(PFVEC2D _vec2d, double _s, const PFVEC2D _v);
extern DLLEXPORT void                 pfAddScaledVec2d(PFVEC2D _vec2d, const PFVEC2D _v1, double _s, const PFVEC2D _v2);
extern DLLEXPORT void                 pfCombineVec2d(PFVEC2D _vec2d, double _a, const PFVEC2D _v1, double _b, const PFVEC2D _v2);
extern DLLEXPORT double               pfSqrDistancePt2d(const PFVEC2D _vec2d, const PFVEC2D _v);
extern DLLEXPORT double               pfNormalizeVec2d(PFVEC2D _vec2d);
extern DLLEXPORT double               pfLengthVec2d(const PFVEC2D _vec2d);
extern DLLEXPORT double               pfDistancePt2d(const PFVEC2D _vec2d, const PFVEC2D _v);

#endif /* !PF_C_API */

#ifdef PF_C_API
/*---------------- pfVec3d CAPI ------------------------------*/

extern DLLEXPORT void                 pfSetVec3d(PFVEC3D _vec3d, double _x, double _y, double _z);
extern DLLEXPORT void                 pfCopyVec3d(PFVEC3D _vec3d, const PFVEC3D _v);
extern DLLEXPORT int                  pfEqualVec3d(const PFVEC3D _vec3d, const PFVEC3D _v);
extern DLLEXPORT int                  pfAlmostEqualVec3d(const PFVEC3D _vec3d, const PFVEC3D _v, double _tol);
extern DLLEXPORT void                 pfNegateVec3d(PFVEC3D _vec3d, const PFVEC3D _v);
extern DLLEXPORT double               pfDotVec3d(const PFVEC3D _vec3d, const PFVEC3D _v);
extern DLLEXPORT void                 pfAddVec3d(PFVEC3D _vec3d, const PFVEC3D _v1, const PFVEC3D _v2);
extern DLLEXPORT void                 pfSubVec3d(PFVEC3D _vec3d, const PFVEC3D _v1, const PFVEC3D _v2);
extern DLLEXPORT void                 pfScaleVec3d(PFVEC3D _vec3d, double _s, const PFVEC3D _v);
extern DLLEXPORT void                 pfAddScaledVec3d(PFVEC3D _vec3d, const PFVEC3D _v1, double _s, const PFVEC3D _v2);
extern DLLEXPORT void                 pfCombineVec3d(PFVEC3D _vec3d, double _a, const PFVEC3D _v1, double _b, const PFVEC3D _v2);
extern DLLEXPORT double               pfSqrDistancePt3d(const PFVEC3D _vec3d, const PFVEC3D _v);
extern DLLEXPORT double               pfNormalizeVec3d(PFVEC3D _vec3d);
extern DLLEXPORT double               pfLengthVec3d(const PFVEC3D _vec3d);
extern DLLEXPORT double               pfDistancePt3d(const PFVEC3D _vec3d, const PFVEC3D _v);
extern DLLEXPORT void                 pfCrossVec3d(PFVEC3D _vec3d, const PFVEC3D _v1, const PFVEC3D _v2);
extern DLLEXPORT void                 pfXformVec3d(PFVEC3D _vec3d, const PFVEC3D _v, const PFMATRIX4D _m);
extern DLLEXPORT void                 pfXformPt3d(PFVEC3D _vec3d, const PFVEC3D _v, const PFMATRIX4D _m);
extern DLLEXPORT void                 pfFullXformPt3d(PFVEC3D _vec3d, const PFVEC3D _v, const PFMATRIX4D _m);
extern DLLEXPORT double               pfFullXformPt3dw(PFVEC3D _vec3d, const PFVEC3D _v, const PFMATRIX4D _m);

#endif /* !PF_C_API */

#ifdef PF_C_API
/*---------------- pfVec4d CAPI ------------------------------*/

extern DLLEXPORT void                 pfSetVec4d(PFVEC4D _vec4d, double _x, double _y, double _z, double _w);
extern DLLEXPORT void                 pfCopyVec4d(PFVEC4D _vec4d, const PFVEC4D _v);
extern DLLEXPORT int                  pfEqualVec4d(const PFVEC4D _vec4d, const PFVEC4D _v);
extern DLLEXPORT int                  pfAlmostEqualVec4d(const PFVEC4D _vec4d, const PFVEC4D _v, double _tol);
extern DLLEXPORT void                 pfNegateVec4d(PFVEC4D _vec4d, const PFVEC4D _v);
extern DLLEXPORT double               pfDotVec4d(const PFVEC4D _vec4d, const PFVEC4D _v);
extern DLLEXPORT void                 pfAddVec4d(PFVEC4D _vec4d, const PFVEC4D _v1, const PFVEC4D _v2);
extern DLLEXPORT void                 pfSubVec4d(PFVEC4D _vec4d, const PFVEC4D _v1, const PFVEC4D _v2);
extern DLLEXPORT void                 pfScaleVec4d(PFVEC4D _vec4d, double _s, const PFVEC4D _v);
extern DLLEXPORT void                 pfAddScaledVec4d(PFVEC4D _vec4d, const PFVEC4D _v1, double _s, const PFVEC4D _v2);
extern DLLEXPORT void                 pfCombineVec4d(PFVEC4D _vec4d, double _a, const PFVEC4D _v1, double _b, const PFVEC4D _v2);
extern DLLEXPORT double               pfSqrDistancePt4d(const PFVEC4D _vec4d, const PFVEC4D _v);
extern DLLEXPORT double               pfNormalizeVec4d(PFVEC4D _vec4d);
extern DLLEXPORT double               pfLengthVec4d(const PFVEC4D _vec4d);
extern DLLEXPORT double               pfDistancePt4d(const PFVEC4D _vec4d, const PFVEC4D _v);
extern DLLEXPORT void                 pfXformVec4d(PFVEC4D _vec4d, const PFVEC4D v, const PFMATRIX4D m);

#endif /* !PF_C_API */

#ifdef PF_C_API
/*---------------- pfMatrix4d CAPI ------------------------------*/

#if 0
#endif
extern DLLEXPORT void                 pfSetMat4d(PFMATRIX4D _mat4d, double *m);
extern DLLEXPORT int                  pfGetMat4dType(const PFMATRIX4D _mat4d);
extern DLLEXPORT void                 pfSetMat4dRowVec3d(PFMATRIX4D _mat4d, int _r, const PFVEC3D _v);
extern DLLEXPORT void                 pfSetMat4dRow(PFMATRIX4D _mat4d, int _r, double _x, double _y, double _z, double _w);
extern DLLEXPORT void                 pfGetMat4dRowVec3d(const PFMATRIX4D _mat4d, int _r, PFVEC3D _dst);
extern DLLEXPORT void                 pfGetMat4dRow(const PFMATRIX4D _mat4d, int _r, double *_x, double *_y, double *_z, double *_w);
extern DLLEXPORT void                 pfSetMat4dColVec3d(PFMATRIX4D _mat4d, int _c, const PFVEC3D _v);
extern DLLEXPORT void                 pfSetMat4dCol(PFMATRIX4D _mat4d, int _c, double _x, double _y, double _z, double _w);
extern DLLEXPORT void                 pfGetMat4dColVec3d(const PFMATRIX4D _mat4d, int _c, PFVEC3D _dst);
extern DLLEXPORT void                 pfGetMat4dCol(const PFMATRIX4D _mat4d, int _c, double *_x, double *_y, double *_z, double *_w);
extern DLLEXPORT void                 pfGetOrthoMat4dCoordd(const PFMATRIX4D _mat4d, pfCoordd* _dst);
extern DLLEXPORT void                 pfMakeIdentMat4d(PFMATRIX4D _mat4d);
extern DLLEXPORT void                 pfMakeEulerMat4d(PFMATRIX4D _mat4d, double _hdeg, double _pdeg, double _rdeg);
extern DLLEXPORT void                 pfMakeRotMat4d(PFMATRIX4D _mat4d, double _degrees, double _x, double _y, double _z);
extern DLLEXPORT void                 pfMakeTransMat4d(PFMATRIX4D _mat4d, double _x, double _y, double _z);
extern DLLEXPORT void                 pfMakeScaleMat4d(PFMATRIX4D _mat4d, double _x, double _y, double _z);
extern DLLEXPORT void                 pfMakeVecRotVecMat4d(PFMATRIX4D _mat4d, const PFVEC3D _v1, const PFVEC3D _v2);
extern DLLEXPORT void                 pfMakeCoorddMat4d(PFMATRIX4D _mat4d, const pfCoordd* _c);
extern DLLEXPORT void                 pfGetOrthoMat4dQuatd(const PFMATRIX4D _mat4d, PFQUATD dst);
extern DLLEXPORT void                 pfMakeQuatMat4d(PFMATRIX4D _mat4d, const PFQUATD q);
extern DLLEXPORT void                 pfCopyMat4d(PFMATRIX4D _mat4d, const PFMATRIX4D _v);
extern DLLEXPORT int                  pfEqualMat4d(const PFMATRIX4D _mat4d, const PFMATRIX4D _m);
extern DLLEXPORT int                  pfAlmostEqualMat4d(const PFMATRIX4D _mat4d, const PFMATRIX4D _m2, double _tol);
extern DLLEXPORT void                 pfTransposeMat4d(PFMATRIX4D _mat4d, PFMATRIX4D _m);
extern DLLEXPORT void                 pfMultMat4d(PFMATRIX4D _mat4d, const PFMATRIX4D _m1, const PFMATRIX4D m2);
extern DLLEXPORT void                 pfAddMat4d(PFMATRIX4D _mat4d, const PFMATRIX4D _m1, const PFMATRIX4D m2);
extern DLLEXPORT void                 pfSubMat4d(PFMATRIX4D _mat4d, const PFMATRIX4D _m1, const PFMATRIX4D m2);
/* pfScaleMat() multiplies full 4X4 by scalar, not a 3D scale xform */
extern DLLEXPORT void                 pfScaleMat4d(PFMATRIX4D _mat4d, double _s, const PFMATRIX4D _m);
extern DLLEXPORT void                 pfPostMultMat4d(PFMATRIX4D _mat4d, const PFMATRIX4D _m);
extern DLLEXPORT void                 pfPreMultMat4d(PFMATRIX4D _mat4d, const PFMATRIX4D _m);
extern DLLEXPORT int                  pfInvertFullMat4d(PFMATRIX4D _mat4d, PFMATRIX4D _m);
extern DLLEXPORT void                 pfInvertAffMat4d(PFMATRIX4D _mat4d, const PFMATRIX4D _m);
extern DLLEXPORT void                 pfInvertOrthoMat4d(PFMATRIX4D _mat4d, const PFMATRIX4D _m);
extern DLLEXPORT void                 pfInvertOrthoNMat4d(PFMATRIX4D _mat4d, PFMATRIX4D _m);
extern DLLEXPORT void                 pfInvertIdentMat4d(PFMATRIX4D _mat4d, const PFMATRIX4D _m);
extern DLLEXPORT void                 pfPreTransMat4d(PFMATRIX4D _mat4d, double _x, double _y, double _z, PFMATRIX4D _m);
extern DLLEXPORT void                 pfPostTransMat4d(PFMATRIX4D _mat4d, const PFMATRIX4D _m, double _x, double _y, double _z);
extern DLLEXPORT void                 pfPreRotMat4d(PFMATRIX4D _mat4d, double _degrees, double _x, double _y, double _z, PFMATRIX4D _m);
extern DLLEXPORT void                 pfPostRotMat4d(PFMATRIX4D _mat4d, const PFMATRIX4D _m, double _degrees, double _x, double _y, double _z);
extern DLLEXPORT void                 pfPreScaleMat4d(PFMATRIX4D _mat4d, double _xs, double _ys, double _zs, PFMATRIX4D _m);
extern DLLEXPORT void                 pfPostScaleMat4d(PFMATRIX4D _mat4d, const PFMATRIX4D _m, double _xs, double _ys, double _zs);

#endif /* !PF_C_API */

#if !PF_CPLUSPLUS_API /* Also in C++ header  EXPORT to CAPI */
/*
 * pfQuatd derives from pfVec4d.
 * pfVec4d routines can also operate on pfQuatds.
 */
#endif /* !PF_CPLUSPLUS_API */

#ifdef PF_C_API
/*---------------- pfQuatd CAPI ------------------------------*/

extern DLLEXPORT void                 pfGetQuatdRot(const PFQUATD _quatd, double *_angle, double *_x, double *_y, double *_z);
extern DLLEXPORT void                 pfMakeRotQuatd(PFQUATD _quatd, double _angle, double _x, double _y, double _z);
extern DLLEXPORT void                 pfRotQuatd(PFQUATD _quatd, PFMATRIX4D m);
extern DLLEXPORT void                 pfMakeMatRotQuatd(PFQUATD _quatd, const PFMATRIX4D m);
extern DLLEXPORT void                 pfMakeVecRotVecQuatd(PFQUATD _quatd, const PFVEC3D rotateFrom, const pfVec3d rotateTo);
extern DLLEXPORT void                 pfConjQuatd(PFQUATD _quatd, const PFQUATD _v);
extern DLLEXPORT double               pfLengthQuatd(const PFQUATD _quatd);
extern DLLEXPORT void                 pfMultQuatd(PFQUATD _quatd, const PFQUATD q1, const PFQUATD q2);
extern DLLEXPORT void                 pfDivQuatd(PFQUATD _quatd, const PFQUATD q1, const PFQUATD q2);
extern DLLEXPORT void                 pfInvertQuatd(PFQUATD _quatd, const PFQUATD q1);
extern DLLEXPORT void                 pfExpQuatd(PFQUATD _quatd, const PFQUATD _q);
extern DLLEXPORT void                 pfLogQuatd(PFQUATD _quatd, const PFQUATD _q);
extern DLLEXPORT void                 pfSlerpQuatd(PFQUATD _quatd, double _t, const PFQUATD _q1, const PFQUATD _q2);
extern DLLEXPORT void                 pfSquadQuatd(PFQUATD _quatd, double _t, const PFQUATD _q1, const PFQUATD _q2, const PFQUATD _a, const PFQUATD _b);
extern DLLEXPORT void                 pfQuatdMeanTangent(PFQUATD _quatd, const PFQUATD _q1, const PFQUATD _q2, const PFQUATD _q3);

#endif /* !PF_C_API */

#if !PF_CPLUSPLUS_API /* Also in C++ header extern "C" { */
extern DLLEXPORT pfMatrix pfIdentMat;
extern DLLEXPORT pfMatrix4d pfIdentMat4d;
#endif /* !PF_CPLUSPLUS_API */

#if !PF_CPLUSPLUS_API /* Also in C++ header  EXPORT to CAPI */

extern DLLEXPORT pfMatrix pfIdentMat;
extern DLLEXPORT pfMatrix4d pfIdentMat4d;

struct DLLEXPORT _pfCoord {
    pfVec3	xyz;
    pfVec3	hpr;
};

struct DLLEXPORT _pfCoordd {
    pfVec3d	xyz;
    pfVec3d	hpr;
};

#endif /* !PF_CPLUSPLUS_API */

#if !PF_CPLUSPLUS_API /* Also in C++ header  EXPORT to CAPI */

#define PF_X	    0
#define PF_Y	    1
#define PF_Z	    2
#define PF_W	    3

#define PF_H	    0	/* Heading */
#define PF_P	    1	/* Pitch */
#define PF_R	    2	/* Roll */

#define PFFP_UNIT_ROUNDOFF	1	/* unit round off */
#define PFFP_ZERO_THRESH	2	/* smaller than this is zero */
#define PFFP_TRAP_FPES		3       /* trap floating point exceptions */

/* ------------------------------ pfMatrix ---------------------------------- */

#define PFMAT_TRANS	0x01	/* something in 4th row */
#define PFMAT_ROT	0x02	/* rot/skew 3x3 includes off-diagonal */
#define PFMAT_SCALE	0x04	/* scaling going on */
#define PFMAT_NONORTHO	0x08	/* 3x3 not orthogonal */
#define PFMAT_PROJ	0x10	/* something in 4th column */
#define PFMAT_HOM_SCALE	0x20	/* [3][3] not 1.0 */
#define PFMAT_MIRROR	0x40	/* 3x3 mirrors */


/* ---------------------------- pfMatStack ---------------------------------- */

#endif /* !PF_CPLUSPLUS_API */

#ifdef PF_C_API
/*---------------- pfMatStack CAPI ------------------------------*/

extern DLLEXPORT pfMatStack*          pfNewMStack(int _size, void *arena);
extern DLLEXPORT pfType*              pfGetMStackClassType(void);
extern DLLEXPORT void                 pfGetMStack(const pfMatStack* mst, PFMATRIX _m);
extern DLLEXPORT pfMatrix*            pfGetMStackTop(const pfMatStack* mst);
extern DLLEXPORT int                  pfGetMStackDepth(const pfMatStack* mst);
extern DLLEXPORT void                 pfResetMStack(pfMatStack* mst);
extern DLLEXPORT int                  pfPushMStack(pfMatStack* mst);
extern DLLEXPORT int                  pfPopMStack(pfMatStack* mst);
extern DLLEXPORT void                 pfLoadMStack(pfMatStack* mst, const PFMATRIX _m);
extern DLLEXPORT void                 pfPreMultMStack(pfMatStack* mst, const PFMATRIX _m);
extern DLLEXPORT void                 pfPostMultMStack(pfMatStack* mst, const PFMATRIX _m);
extern DLLEXPORT void                 pfPreTransMStack(pfMatStack* mst, float _x, float _y, float _z);
extern DLLEXPORT void                 pfPostTransMStack(pfMatStack* mst, float _x, float _y, float _z);
extern DLLEXPORT void                 pfPreRotMStack(pfMatStack* mst, float _degrees, float _x, float _y, float _z);
extern DLLEXPORT void                 pfPostRotMStack(pfMatStack* mst, float _degrees, float _x, float _y, float _z);
extern DLLEXPORT void                 pfPreScaleMStack(pfMatStack* mst, float _xs, float _ys, float _zs);
extern DLLEXPORT void                 pfPostScaleMStack(pfMatStack* mst, float _xs, float _ys, float _zs);

#endif /* !PF_C_API */

#if !PF_CPLUSPLUS_API /* Also in C++ header  EXPORT to CAPI */

/* ------------------------------------------------------------------------ */
/* ------------------------------ Macros ---------------------------------- */
/* ------------------------------------------------------------------------ */

/*
 * 2 Vectors
 */

#define PFSET_VEC2(_dst, _x, _y) \
    (((_dst)[0] = (_x)), \
     ((_dst)[1] = (_y)))

#define PFGET_VEC2(_src, _x, _y) \
    ((*(_x) = (_src)[0]), \
     (*(_y) = (_src)[1]))

#define PFCOPY_VEC2(_dst, _v) \
    (((_dst)[0] = (_v)[0]),     \
     ((_dst)[1] = (_v)[1]))

#define PFSCALE_VEC2(_dst, _s, _v) \
    (((_dst)[0] = (_s) * (_v)[0]),     \
     ((_dst)[1] = (_s) * (_v)[1]))

#define PFADD_VEC2(_dst, _v1, _v2)     \
    (((_dst)[0] = (_v1)[0] + (_v2)[0]), \
     ((_dst)[1] = (_v1)[1] + (_v2)[1]))

#define PFSUB_VEC2(_dst, _v1, _v2)     \
    (((_dst)[0] = (_v1)[0] - (_v2)[0]), \
     ((_dst)[1] = (_v1)[1] - (_v2)[1]))

#define PFCOMBINE_VEC2(_dst, _s1, _v1, _s2, _v2)   \
    (((_dst)[0] = (_s1)*(_v1)[0] + (_s2)*(_v2)[0]), \
     ((_dst)[1] = (_s1)*(_v1)[1] + (_s2)*(_v2)[1]))

#define PFADD_SCALED_VEC2(_dst, _v1, _s2, _v2)   \
    (((_dst)[0] = (_v1)[0] + (_s2)*(_v2)[0]), \
     ((_dst)[1] = (_v1)[1] + (_s2)*(_v2)[1]))

#define PFDOT_VEC2(_v0, _v1) \
    ((_v0)[0] * (_v1)[0] + \
     (_v0)[1] * (_v1)[1])

#define PFLENGTH_VEC2(_v) \
    (pfSqrt(PFDOT_VEC2((_v), (_v))))
#define PFLENGTH_VEC2D(_v) \
    (sqrt(PFDOT_VEC2((_v), (_v))))

#define PFSQR_DISTANCE_PT2(_v1, _v2) \
    (PF_SQUARE((_v1)[0]-(_v2)[0]) + \
     PF_SQUARE((_v1)[1]-(_v2)[1]))

#define PFDISTANCE_PT2(_v1, _v2) \
    (pfSqrt(PFSQR_DISTANCE_PT2((_v1), (_v2))))
#define PFDISTANCE_PT2D(_v1, _v2) \
    (sqrt(PFSQR_DISTANCE_PT2((_v1), (_v2))))

#define PFNEGATE_VEC2(_dst, _v) \
    (((_dst)[0] = -(_v)[0]),     \
     ((_dst)[1] = -(_v)[1]))

#define PFEQUAL_VEC2(_v1, _v2)	\
	((_v1)[0] == (_v2)[0] && \
	 (_v1)[1] == (_v2)[1])

#define PFALMOST_EQUAL_VEC2(_v1, _v2, tol)				\
((_v1)[0] >= (_v2)[0] - (tol) && (_v1)[0] <= (_v2)[0] + (tol) &&	\
 (_v1)[1] >= (_v2)[1] - (tol) && (_v1)[1] <= (_v2)[1] + (tol))

/*
 * 3 Vectors
 */

#define PFSET_VEC3(_dst, _x, _y, _z) \
    (((_dst)[0] = (_x)), \
     ((_dst)[1] = (_y)), \
     ((_dst)[2] = (_z)))

#define PFGET_VEC3(_src, _x, _y, _z) \
    ((*(_x) = (_src)[0]), \
     (*(_y) = (_src)[1]), \
     (*(_z) = (_src)[2]))

#define PFCOPY_VEC3(_dst, _v) \
    (((_dst)[0] = (_v)[0]),     \
     ((_dst)[1] = (_v)[1]),     \
     ((_dst)[2] = (_v)[2]))

#define PFNEGATE_VEC3(_dst, _v) \
    (((_dst)[0] = -(_v)[0]),     \
     ((_dst)[1] = -(_v)[1]),     \
     ((_dst)[2] = -(_v)[2]))

#define PFADD_VEC3(_dst, _v1, _v2)     \
    (((_dst)[0] = (_v1)[0] + (_v2)[0]), \
     ((_dst)[1] = (_v1)[1] + (_v2)[1]), \
     ((_dst)[2] = (_v1)[2] + (_v2)[2]))

#define PFSUB_VEC3(_dst, _v1, _v2)     \
    (((_dst)[0] = (_v1)[0] - (_v2)[0]), \
     ((_dst)[1] = (_v1)[1] - (_v2)[1]), \
     ((_dst)[2] = (_v1)[2] - (_v2)[2]))

#define PFCOMBINE_VEC3(_dst, _s1, _v1, _s2, _v2)   \
    (((_dst)[0] = (_s1)*(_v1)[0] + (_s2)*(_v2)[0]), \
     ((_dst)[1] = (_s1)*(_v1)[1] + (_s2)*(_v2)[1]), \
     ((_dst)[2] = (_s1)*(_v1)[2] + (_s2)*(_v2)[2]))

#define PFADD_SCALED_VEC3(_dst, _v1, _s2, _v2)   \
    (((_dst)[0] = (_v1)[0] + (_s2)*(_v2)[0]), \
     ((_dst)[1] = (_v1)[1] + (_s2)*(_v2)[1]), \
     ((_dst)[2] = (_v1)[2] + (_s2)*(_v2)[2]))

#define PFSCALE_VEC3(_dst, _s, _v) \
    (((_dst)[0] = (_s) * (_v)[0]),     \
     ((_dst)[1] = (_s) * (_v)[1]),     \
     ((_dst)[2] = (_s) * (_v)[2]))

#define PFDOT_VEC3(_v0, _v1) \
    ((_v0)[0] * (_v1)[0] +    \
     (_v0)[1] * (_v1)[1] +    \
     (_v0)[2] * (_v1)[2])

#define PFLENGTH_VEC3(_v) \
    (pfSqrt(PFDOT_VEC3((_v), (_v))))
#define PFLENGTH_VEC3D(_v) \
    (sqrt(PFDOT_VEC3((_v), (_v))))

#define PFSQR_DISTANCE_PT3(_v1, _v2) \
    (PF_SQUARE((_v1)[0]-(_v2)[0]) + \
     PF_SQUARE((_v1)[1]-(_v2)[1]) + \
     PF_SQUARE((_v1)[2]-(_v2)[2]))

#define PFDISTANCE_PT3(_v1, _v2) \
    (pfSqrt(PFSQR_DISTANCE_PT3((_v1), (_v2))))
#define PFDISTANCE_PT3D(_v1, _v2) \
    (sqrt(PFSQR_DISTANCE_PT3((_v1), (_v2))))

#define PFEQUAL_VEC3(_v1, _v2)	\
	((_v1)[0] == (_v2)[0] && \
	 (_v1)[1] == (_v2)[1] && \
	 (_v1)[2] == (_v2)[2])

#define PFALMOST_EQUAL_VEC3(_v1, _v2, tol)			\
((_v1)[0] >= (_v2)[0] - (tol) && (_v1)[0] <= (_v2)[0] + (tol) &&	\
 (_v1)[1] >= (_v2)[1] - (tol) && (_v1)[1] <= (_v2)[1] + (tol) &&	\
 (_v1)[2] >= (_v2)[2] - (tol) && (_v1)[2] <= (_v2)[2] + (tol))

/*
 * 4 Vectors -- Ordinary non homogenous coords
 */

#define PFSET_VEC4(_dst, _x, _y, _z, _w) \
    ((((_dst))[0] = (_x)), \
     (((_dst))[1] = (_y)), \
     (((_dst))[2] = (_z)), \
     (((_dst))[3] = (_w)))

#define PFGET_VEC4(_src, _x, _y, _z, _w) \
    ((*(_x) = ((_src))[0]), \
     (*(_y) = ((_src))[1]), \
     (*(_z) = ((_src))[2]), \
     (*(_w) = ((_src))[3]))

#define PFCOPY_VEC4(_dst, _v) \
    ((((_dst))[0] = ((_v))[0]), \
     (((_dst))[1] = ((_v))[1]), \
     (((_dst))[2] = ((_v))[2]), \
     (((_dst))[3] = ((_v))[3]))

#define PFNEGATE_VEC4(_dst, _v) \
    ((((_dst))[0] = -((_v))[0]), \
     (((_dst))[1] = -((_v))[1]), \
     (((_dst))[2] = -((_v))[2]), \
     (((_dst))[3] = -((_v))[3]))

#define PFADD_VEC4(_dst, _v1, _v2)     \
    ((((_dst))[0] = ((_v1))[0] + ((_v2))[0]), \
     (((_dst))[1] = ((_v1))[1] + ((_v2))[1]), \
     (((_dst))[2] = ((_v1))[2] + ((_v2))[2]), \
     (((_dst))[3] = ((_v1))[3] + ((_v2))[3]))

#define PFSUB_VEC4(_dst, _v1, _v2)     \
    ((((_dst))[0] = ((_v1))[0] - ((_v2))[0]), \
     (((_dst))[1] = ((_v1))[1] - ((_v2))[1]), \
     (((_dst))[2] = ((_v1))[2] - ((_v2))[2]), \
     (((_dst))[3] = ((_v1))[3] - ((_v2))[3]))

#define PFSCALE_VEC4(_dst, _s, _v) \
    ((((_dst))[0] = (_s) * ((_v))[0]), \
     (((_dst))[1] = (_s) * ((_v))[1]), \
     (((_dst))[2] = (_s) * ((_v))[2]), \
     (((_dst))[3] = (_s) * ((_v))[3]))

#define PFADD_SCALED_VEC4(_dst, _v1, _s2, _v2)   \
    ((((_dst))[0] = ((_v1))[0] + (_s2)*((_v2))[0]), \
     (((_dst))[1] = ((_v1))[1] + (_s2)*((_v2))[1]), \
     (((_dst))[2] = ((_v1))[2] + (_s2)*((_v2))[2]), \
     (((_dst))[3] = ((_v1))[3] + (_s2)*((_v2))[3]))

#define PFCOMBINE_VEC4(_dst, _s1, _v1, _s2, _v2)   \
    ((((_dst))[0] = (_s1)*((_v1))[0] + (_s2)*((_v2))[0]), \
     (((_dst))[1] = (_s1)*((_v1))[1] + (_s2)*((_v2))[1]), \
     (((_dst))[2] = (_s1)*((_v1))[2] + (_s2)*((_v2))[2]), \
     (((_dst))[3] = (_s1)*((_v1))[3] + (_s2)*((_v2))[3]))

#define PFDOT_VEC4(_v0, _v1) \
    ((_v0)[0] * ((_v1))[0] + \
     (_v0)[1] * ((_v1))[1] + \
     (_v0)[2] * ((_v1))[2] + \
     (_v0)[3] * ((_v1))[3])

#define PFLENGTH_VEC4(_v) \
    (pfSqrt(PFDOT_VEC4((_v), (_v))))
#define PFLENGTH_VEC4D(_v) \
    (sqrt(PFDOT_VEC4((_v), (_v))))

#define PFSQR_DISTANCE_PT4(_v1, _v2) \
    (PF_SQUARE(((_v1))[0]-((_v2))[0]) + \
     PF_SQUARE(((_v1))[1]-((_v2))[1]) + \
     PF_SQUARE(((_v1))[2]-((_v2))[2]) + \
     PF_SQUARE(((_v1))[3]-((_v2))[3]))

#define PFDISTANCE_PT4(_v1, _v2) \
    (pfSqrt(PFSQR_DISTANCE_PT4((_v1), (_v2))))
#define PFDISTANCE_PT4D(_v1, _v2) \
    (sqrt(PFSQR_DISTANCE_PT4((_v1), (_v2))))

#define PFEQUAL_VEC4(_v1, _v2)	\
	(((_v1))[0] == ((_v2))[0] && \
	 ((_v1))[1] == ((_v2))[1] && \
	 ((_v1))[2] == ((_v2))[2] && \
	 ((_v1))[3] == ((_v2))[3])

#define PFALMOST_EQUAL_VEC4(_v1, _v2, tol)			\
(((_v1))[0] >= ((_v2))[0] - (tol) &&  \
 ((_v1))[0] <= ((_v2))[0] + (tol) &&	\
 ((_v1))[1] >= ((_v2))[1] - (tol) &&  \
 ((_v1))[1] <= ((_v2))[1] + (tol) &&	\
 ((_v1))[2] >= ((_v2))[2] - (tol) &&  \
 ((_v1))[2] <= ((_v2))[2] + (tol) &&	\
 ((_v1))[3] >= ((_v2))[3] - (tol) &&  \
 ((_v1))[3] <= ((_v2))[3] + (tol))
        
/*
 * 4X4 Matrices
 */

#define PFMAKE_IDENT_MAT(_dst) \
   (((_dst)[0][0] = 1), \
    ((_dst)[0][1] = 0), \
    ((_dst)[0][2] = 0), \
    ((_dst)[0][3] = 0), \
    ((_dst)[1][0] = 0), \
    ((_dst)[1][1] = 1), \
    ((_dst)[1][2] = 0), \
    ((_dst)[1][3] = 0), \
    ((_dst)[2][0] = 0), \
    ((_dst)[2][1] = 0), \
    ((_dst)[2][2] = 1), \
    ((_dst)[2][3] = 0), \
    ((_dst)[3][0] = 0), \
    ((_dst)[3][1] = 0), \
    ((_dst)[3][2] = 0), \
    ((_dst)[3][3] = 1))

#define PFCOPY_MAT(_dst, _m)      \
   (((_dst)[0][0] = (_m)[0][0]), \
    ((_dst)[0][1] = (_m)[0][1]), \
    ((_dst)[0][2] = (_m)[0][2]), \
    ((_dst)[0][3] = (_m)[0][3]), \
    ((_dst)[1][0] = (_m)[1][0]), \
    ((_dst)[1][1] = (_m)[1][1]), \
    ((_dst)[1][2] = (_m)[1][2]), \
    ((_dst)[1][3] = (_m)[1][3]), \
    ((_dst)[2][0] = (_m)[2][0]), \
    ((_dst)[2][1] = (_m)[2][1]), \
    ((_dst)[2][2] = (_m)[2][2]), \
    ((_dst)[2][3] = (_m)[2][3]), \
    ((_dst)[3][0] = (_m)[3][0]), \
    ((_dst)[3][1] = (_m)[3][1]), \
    ((_dst)[3][2] = (_m)[3][2]), \
    ((_dst)[3][3] = (_m)[3][3]))

#define PFEQUAL_MAT(_m1, _m2)      \
   (((_m1)[0][0] == (_m2)[0][0]) && \
    ((_m1)[0][1] == (_m2)[0][1]) && \
    ((_m1)[0][2] == (_m2)[0][2]) && \
    ((_m1)[0][3] == (_m2)[0][3]) && \
    ((_m1)[1][0] == (_m2)[1][0]) && \
    ((_m1)[1][1] == (_m2)[1][1]) && \
    ((_m1)[1][2] == (_m2)[1][2]) && \
    ((_m1)[1][3] == (_m2)[1][3]) && \
    ((_m1)[2][0] == (_m2)[2][0]) && \
    ((_m1)[2][1] == (_m2)[2][1]) && \
    ((_m1)[2][2] == (_m2)[2][2]) && \
    ((_m1)[2][3] == (_m2)[2][3]) && \
    ((_m1)[3][0] == (_m2)[3][0]) && \
    ((_m1)[3][1] == (_m2)[3][1]) && \
    ((_m1)[3][2] == (_m2)[3][2]) && \
    ((_m1)[3][3] == (_m2)[3][3]))

#define PFALMOST_EQUAL_MAT(_m1, _m2, tol)			\
((_m1)[0][0] >= (_m2)[0][0] - (tol) && (_m1)[0][0] <= (_m2)[0][0] + (tol) && \
 (_m1)[0][1] >= (_m2)[0][1] - (tol) && (_m1)[0][1] <= (_m2)[0][1] + (tol) && \
 (_m1)[0][2] >= (_m2)[0][2] - (tol) && (_m1)[0][2] <= (_m2)[0][2] + (tol) && \
 (_m1)[0][3] >= (_m2)[0][3] - (tol) && (_m1)[0][3] <= (_m2)[0][3] + (tol) && \
 (_m1)[1][0] >= (_m2)[1][0] - (tol) && (_m1)[1][0] <= (_m2)[1][0] + (tol) && \
 (_m1)[1][1] >= (_m2)[1][1] - (tol) && (_m1)[1][1] <= (_m2)[1][1] + (tol) && \
 (_m1)[1][2] >= (_m2)[1][2] - (tol) && (_m1)[1][2] <= (_m2)[1][2] + (tol) && \
 (_m1)[1][3] >= (_m2)[1][3] - (tol) && (_m1)[1][3] <= (_m2)[1][3] + (tol) && \
 (_m1)[2][0] >= (_m2)[2][0] - (tol) && (_m1)[2][0] <= (_m2)[2][0] + (tol) && \
 (_m1)[2][1] >= (_m2)[2][1] - (tol) && (_m1)[2][1] <= (_m2)[2][1] + (tol) && \
 (_m1)[2][2] >= (_m2)[2][2] - (tol) && (_m1)[2][2] <= (_m2)[2][2] + (tol) && \
 (_m1)[2][3] >= (_m2)[2][3] - (tol) && (_m1)[2][3] <= (_m2)[2][3] + (tol) && \
 (_m1)[3][0] >= (_m2)[3][0] - (tol) && (_m1)[3][0] <= (_m2)[3][0] + (tol) && \
 (_m1)[3][1] >= (_m2)[3][1] - (tol) && (_m1)[3][1] <= (_m2)[3][1] + (tol) && \
 (_m1)[3][2] >= (_m2)[3][2] - (tol) && (_m1)[3][2] <= (_m2)[3][2] + (tol) && \
 (_m1)[3][3] >= (_m2)[3][3] - (tol) && (_m1)[3][3] <= (_m2)[3][3] + (tol))

#define PFMAKE_TRANS_MAT(_dst, _x, _y, _z) \
   (((_dst)[0][0] = 1), \
    ((_dst)[0][1] = 0), \
    ((_dst)[0][2] = 0), \
    ((_dst)[0][3] = 0), \
    ((_dst)[1][0] = 0), \
    ((_dst)[1][1] = 1), \
    ((_dst)[1][2] = 0), \
    ((_dst)[1][3] = 0), \
    ((_dst)[2][0] = 0), \
    ((_dst)[2][1] = 0), \
    ((_dst)[2][2] = 1), \
    ((_dst)[2][3] = 0), \
    ((_dst)[3][0] = (_x)), \
    ((_dst)[3][1] = (_y)), \
    ((_dst)[3][2] = (_z)), \
    ((_dst)[3][3] = 1))

#define PFMAKE_SCALE_MAT(_dst, _s1, _s2, _s3) \
   (((_dst)[0][0] = (_s1)), \
    ((_dst)[0][1] = 0), \
    ((_dst)[0][2] = 0), \
    ((_dst)[0][3] = 0), \
    ((_dst)[1][0] = 0), \
    ((_dst)[1][1] = (_s2)), \
    ((_dst)[1][2] = 0), \
    ((_dst)[1][3] = 0), \
    ((_dst)[2][0] = 0), \
    ((_dst)[2][1] = 0), \
    ((_dst)[2][2] = (_s3)), \
    ((_dst)[2][3] = 0), \
    ((_dst)[3][0] = 0), \
    ((_dst)[3][1] = 0), \
    ((_dst)[3][2] = 0), \
    ((_dst)[3][3] = 1))

#define PFGET_MAT_ROWVEC3(_m, _r, _v) \
    (((_v)[0] = (_m)[(_r)][0]), \
     ((_v)[1] = (_m)[(_r)][1]), \
     ((_v)[2] = (_m)[(_r)][2]))

#define PFSET_MAT_ROWVEC3(_m, _r, _v) \
    (((_m)[(_r)][0] = ((_v)[0])), \
     ((_m)[(_r)][1] = ((_v)[1])), \
     ((_m)[(_r)][2] = ((_v)[2])))

#define PFGET_MAT_COLVEC3(_m, _c, _v) \
    (((_v)[0] = (_m)[0][(_c)]), \
     ((_v)[1] = (_m)[1][(_c)]), \
     ((_v)[2] = (_m)[2][(_c)]))

#define PFSET_MAT_COLVEC3(_m, _c, _v) \
    (((_m)[0][(_c)] = ((_v)[0])), \
     ((_m)[1][(_c)] = ((_v)[1])), \
     ((_m)[2][(_c)] = ((_v)[2])))

#define PFGET_MAT_ROW(_m, _r, _x, _y, _z, _w) \
    ((*(_x) = (_m)[(_r)][0]), \
     (*(_y) = (_m)[(_r)][1]), \
     (*(_z) = (_m)[(_r)][2]), \
     (*(_w) = (_m)[(_r)][3])

#define PFSET_MAT_ROW(_m, _r, _x, _y, _z, _w) \
    (((_m)[(_r)][0] = ((_x))), \
     ((_m)[(_r)][1] = ((_y))), \
     ((_m)[(_r)][2] = ((_z))), \
     ((_m)[(_r)][3] = ((_w))))

#define PFGET_MAT_COL(_m, _c, _x, _y, _z, _w) \
    ((*(_x) = (_m)[0][(_c)]), \
     (*(_y) = (_m)[1][(_c)]), \
     (*(_z) = (_m)[2][(_c)]), \
     (*(_w) = (_m)[3][(_c)]))

#define PFSET_MAT_COL(_m, _c, _x, _y, _z, _w) \
    (((_m)[0][(_c)] = ((_x))), \
     ((_m)[1][(_c)] = ((_y))), \
     ((_m)[2][(_c)] = ((_z))), \
     ((_m)[3][(_c)] = ((_w))))

/* ------------------------------------------------------------------------ */
/* -------------------------- pfQuat Macros ------------------------------- */
/* ------------------------------------------------------------------------ */

/*
 * This implementation follows the lead of the greatest proponent
 * and expositor of quaternions in modern times, Ken Shoemake. To
 * develop an appreciation of the finer points, especially of the
 * slerp and squad operations, refer to his two SIGGRAPH tutorial 
 * papers:
 *
 *   "Animating Rotation with Quaternion Curves"
 *      SIGGRAPH Proceedings Vol 19, Number 3, 1985
 *
 *   "Quaternion Calculus For Animation"
 *      SIGGRAPH course notes 1989
 *
 * These routines implement the quaternion xi + yj + zk + w using
 * a pfVec4&  to store the values {x,y,z,w}.
 */

/* the almost-zero floating-point tolerance used to guard divisions */
#define	PFQUAT_EPS	0.00001f

/*
 *	macro definitions
 */

#define PFLENGTH_QUAT(_q) \
    (PFDOT_VEC4((_q), (_q)))

#define PFCONJ_QUAT(_dst, _q) \
    (((_dst)[PF_X] = -(_q)[PF_X]), \
     ((_dst)[PF_Y] = -(_q)[PF_Y]), \
     ((_dst)[PF_Z] = -(_q)[PF_Z]), \
     ((_dst)[PF_W] =  (_q)[PF_W]))

#define PFMULT_QUAT(_dst, _q1, _q2) \
    do { \
	pfQuat _q; \
	_q[PF_X] = (_q1)[PF_W]*(_q2)[PF_X] + (_q1)[PF_X]*(_q2)[PF_W] + \
		   (_q1)[PF_Z]*(_q2)[PF_Y] - (_q1)[PF_Y]*(_q2)[PF_Z];  \
	_q[PF_Y] = (_q1)[PF_W]*(_q2)[PF_Y] + (_q1)[PF_Y]*(_q2)[PF_W] + \
		   (_q1)[PF_X]*(_q2)[PF_Z] - (_q1)[PF_Z]*(_q2)[PF_X];  \
	_q[PF_Z] = (_q1)[PF_W]*(_q2)[PF_Z] + (_q1)[PF_Z]*(_q2)[PF_W] + \
		   (_q1)[PF_Y]*(_q2)[PF_X] - (_q1)[PF_X]*(_q2)[PF_Y];  \
	_q[PF_W] = (_q1)[PF_W]*(_q2)[PF_W] - (_q1)[PF_X]*(_q2)[PF_X] - \
		   (_q1)[PF_Y]*(_q2)[PF_Y] - (_q1)[PF_Z]*(_q2)[PF_Z];  \
	(_dst)[PF_X] = _q[PF_X]; \
	(_dst)[PF_Y] = _q[PF_Y]; \
	(_dst)[PF_Z] = _q[PF_Z]; \
	(_dst)[PF_W] = _q[PF_W]; \
    } while (0)


#define PFDIV_QUAT(_dst, _q1, _q2) \
    do { \
	pfQuat _inv; \
	PFINVERT_QUAT(_inv, (_q2)); \
	PFMULT_QUAT((_dst), (_q1), _inv); \
    } while (0)

#define PFINVERT_QUAT(_dst, _q) \
    do { \
	float _length = PFLENGTH_QUAT((_q)); \
	float _invLength; \
	if (_length >= PFQUAT_EPS) \
	{ \
	    _invLength = 1.0f/_length; \
	    (_dst)[PF_X] = -(_q)[PF_X]*_invLength; \
	    (_dst)[PF_Y] = -(_q)[PF_Y]*_invLength; \
	    (_dst)[PF_Z] = -(_q)[PF_Z]*_invLength; \
	    (_dst)[PF_W] =  (_q)[PF_W]*_invLength; \
	} \
	else \
	    PFSET_VEC4((_dst), 0.0f, 0.0f, 0.0f, 0.0f); \
    } while (0)

#endif /* !PF_CPLUSPLUS_API */


#ifdef PF_C_API
/*---------------- pfSeg CAPI ------------------------------*/

extern DLLEXPORT void                 pfMakePtsSeg(pfSeg* _seg, const PFVEC3 p1, const PFVEC3 p2);
extern DLLEXPORT void                 pfMakePolarSeg(pfSeg* _seg, const PFVEC3 _pos, float azi, float elev, float len);
extern DLLEXPORT void                 pfClipSeg(pfSeg* _seg, const pfSeg *seg, float d1, float d2);
extern DLLEXPORT int                  pfClosestPtsOnSeg(const pfSeg* _seg, const pfSeg *seg, PFVEC3 dst1, PFVEC3 dst2);
extern DLLEXPORT void                 pfFromSegdSeg(pfSeg* _seg, const pfSegd *segd);

#endif /* !PF_C_API */

#ifdef PF_C_API
/*---------------- pfSegd CAPI ------------------------------*/

extern DLLEXPORT void                 pfMakePts(pfSegd* _segd, const PFVEC3 p1, const PFVEC3 p2);
extern DLLEXPORT void                 pfMakePtsd(pfSegd* _segd, const PFVEC3D p1, const PFVEC3D p2);
extern DLLEXPORT void                 pfMakePolar(pfSegd* _segd, const PFVEC3 _pos, float azi, float elev, float len);
extern DLLEXPORT void                 pfMakePolard(pfSegd* _segd, const PFVEC3D _pos, float azi, float elev, float len);
extern DLLEXPORT void                 pfClipSegd(pfSegd* _segd, const pfSegd *seg, float d1, float d2);
extern DLLEXPORT int                  pfClosestPtsOnSegd(const pfSegd* _segd, const pfSegd *seg, PFVEC3 dst1, PFVEC3 dst2);
extern DLLEXPORT int                  pfClosestPtsOnd(const pfSegd* _segd, const pfSegd *seg, PFVEC3D dst1, PFVEC3D dst2);
extern DLLEXPORT void                 pfFromSegSegd(pfSegd* _segd, const pfSeg *seg);

#endif /* !PF_C_API */

#ifdef PF_C_API
/*---------------- pfPlane CAPI ------------------------------*/

extern DLLEXPORT void                 pfMakePtsPlane(pfPlane* _plane, const PFVEC3 p1, const PFVEC3 p2, const PFVEC3 p3);
extern DLLEXPORT void                 pfMakeNormPtPlane(pfPlane* _plane, const PFVEC3 _norm, const PFVEC3 _pos);
extern DLLEXPORT void                 pfDisplacePlane(pfPlane* _plane, float _d);
extern DLLEXPORT int                  pfHalfSpaceContainsBox(const pfPlane* _plane, const pfBox *box);
extern DLLEXPORT int                  pfHalfSpaceContainsSphere(const pfPlane* _plane, const pfSphere *sph);
extern DLLEXPORT int                  pfHalfSpaceContainsCyl(const pfPlane* _plane, const pfCylinder *cyl);
extern DLLEXPORT int                  pfHalfSpaceContainsPt(const pfPlane* _plane, const PFVEC3 pt);
extern DLLEXPORT void                 pfOrthoXformPlane(pfPlane* _plane, const pfPlane *pln, const PFMATRIX m);
extern DLLEXPORT void                 pfClosestPtOnPlane(const pfPlane* _plane, const PFVEC3 pt, PFVEC3 dst);
extern DLLEXPORT int                  pfPlaneIsectSeg(const pfPlane* _plane, const pfSeg *seg, float *d);
extern DLLEXPORT int                  pfPlaneIsectSegd(const pfPlane* _plane, const pfSegd *seg, double *d);
extern DLLEXPORT int                  pfHalfSpaceIsectSeg(const pfPlane* _plane, const pfSeg *seg, float *d1, float *d2);
extern DLLEXPORT int                  pfHalfSpaceIsectSegd(const pfPlane* _plane, const pfSegd *seg, double *d1, double *d2);
extern DLLEXPORT int                  pfClipConvexPolygonPlane(pfPlane* _plane, int nVerts, pfVec3 verts[], int texdim, float texcoords[]);

#endif /* !PF_C_API */

#ifdef PF_C_API
/*---------------- pfSphere CAPI ------------------------------*/

extern DLLEXPORT void                 pfMakeEmptySphere(pfSphere* _sphere);
extern DLLEXPORT int                  pfSphereContainsPt(const pfSphere* _sphere, const PFVEC3 pt);
extern DLLEXPORT int                  pfSphereContainsSphere(const pfSphere* _sphere, const pfSphere *_sph);
extern DLLEXPORT int                  pfSphereContainsCyl(const pfSphere* _sphere, const pfCylinder *cyl);
extern DLLEXPORT int                  pfSphereContainsCyld(const pfSphere* _sphere, const pfCylinderd *cyl);
extern DLLEXPORT int                  pfSphereContainsBox(const pfSphere* _sphere, const pfBox *box);
extern DLLEXPORT void                 pfSphereAroundPts(pfSphere* _sphere, const pfVec3* pts, int npt);
extern DLLEXPORT void                 pfSphereAroundSpheres(pfSphere* _sphere, const pfSphere **sphs, int nsph);
extern DLLEXPORT void                 pfSphereAroundBoxes(pfSphere* _sphere, const pfBox **boxes, int nbox);
extern DLLEXPORT void                 pfSphereAroundCyls(pfSphere* _sphere, const pfCylinder **cyls, int ncyl);
extern DLLEXPORT void                 pfSphereExtendByPt(pfSphere* _sphere, const PFVEC3 pt);
extern DLLEXPORT void                 pfSphereExtendBySphere(pfSphere* _sphere, const pfSphere *sph);
extern DLLEXPORT void                 pfSphereExtendByCyl(pfSphere* _sphere, const pfCylinder *cyl);
extern DLLEXPORT void                 pfOrthoXformSphere(pfSphere* _sphere, const pfSphere *sph, const PFMATRIX m);
extern DLLEXPORT int                  pfSphereIsectSeg(const pfSphere* _sphere, const pfSeg *seg, float *d1, float *d2);

#endif /* !PF_C_API */

#ifdef PF_C_API
/*---------------- pfSpheredf CAPI ------------------------------*/

extern DLLEXPORT void                 pfMakeEmptySpheredf(pfSpheredf* _spheredf);
extern DLLEXPORT int                  pfSpheredfContainsPt(const pfSpheredf* _spheredf, const PFVEC3 pt);
extern DLLEXPORT int                  pfSpheredfContainsSpheredf(const pfSpheredf* _spheredf, const pfSpheredf *_sph);
extern DLLEXPORT int                  pfSpheredfContainsCyl(const pfSpheredf* _spheredf, const pfCylinder *cyl);
extern DLLEXPORT int                  pfSpheredfContainsCyld(const pfSpheredf* _spheredf, const pfCylinderd *cyl);
extern DLLEXPORT int                  pfSpheredfContainsBox(const pfSpheredf* _spheredf, const pfBox *box);
extern DLLEXPORT void                 pfSpheredfAroundPts(pfSpheredf* _spheredf, const pfVec3* pts, int npt);
extern DLLEXPORT void                 pfSpheredfAroundSpheredfs(pfSpheredf* _spheredf, const pfSpheredf **sphs, int nsph);
extern DLLEXPORT void                 pfSpheredfAroundBoxes(pfSpheredf* _spheredf, const pfBox **boxes, int nbox);
extern DLLEXPORT void                 pfSpheredfAroundCyls(pfSpheredf* _spheredf, const pfCylinder **cyls, int ncyl);
extern DLLEXPORT void                 pfSpheredfExtendByPt(pfSpheredf* _spheredf, const PFVEC3 pt);
extern DLLEXPORT void                 pfSpheredfExtendBySpheredf(pfSpheredf* _spheredf, const pfSpheredf *sph);
extern DLLEXPORT void                 pfSpheredfExtendByCyl(pfSpheredf* _spheredf, const pfCylinder *cyl);
extern DLLEXPORT void                 pfOrthoXformSpheredf(pfSpheredf* _spheredf, const pfSpheredf *sph, const PFMATRIX4D m);

#endif /* !PF_C_API */

#ifdef PF_C_API
/*---------------- pfSpheredd CAPI ------------------------------*/

extern DLLEXPORT void                 pfMakeEmptySpheredd(pfSpheredd* _spheredd);
extern DLLEXPORT int                  pfSphereddContainsPt(const pfSpheredd* _spheredd, const PFVEC3 pt);
extern DLLEXPORT int                  pfSphereddContainsSpheredd(const pfSpheredd* _spheredd, const pfSpheredd *_sph);
extern DLLEXPORT int                  pfSphereddContainsCyl(const pfSpheredd* _spheredd, const pfCylinder *cyl);
extern DLLEXPORT int                  pfSphereddContainsCyld(const pfSpheredd* _spheredd, const pfCylinderd *cyl);
extern DLLEXPORT int                  pfSphereddContainsBox(const pfSpheredd* _spheredd, const pfBox *box);
extern DLLEXPORT void                 pfSphereddAroundPts(pfSpheredd* _spheredd, const pfVec3* pts, int npt);
extern DLLEXPORT void                 pfSphereddAroundSpheredds(pfSpheredd* _spheredd, const pfSpheredd **sphs, int nsph);
extern DLLEXPORT void                 pfSphereddAroundBoxes(pfSpheredd* _spheredd, const pfBox **boxes, int nbox);
extern DLLEXPORT void                 pfSphereddAroundCyls(pfSpheredd* _spheredd, const pfCylinder **cyls, int ncyl);
extern DLLEXPORT void                 pfSphereddExtendByPt(pfSpheredd* _spheredd, const PFVEC3 pt);
extern DLLEXPORT void                 pfSphereddExtendBySpheredd(pfSpheredd* _spheredd, const pfSpheredd *sph);
extern DLLEXPORT void                 pfSphereddExtendByCyl(pfSpheredd* _spheredd, const pfCylinder *cyl);
extern DLLEXPORT void                 pfOrthoXformSpheredd(pfSpheredd* _spheredd, const pfSpheredd *sph, const PFMATRIX4D m);

#endif /* !PF_C_API */

#ifdef PF_C_API
/*---------------- pfCylinder CAPI ------------------------------*/

extern DLLEXPORT void                 pfMakeEmptyCyl(pfCylinder* _cyl);
extern DLLEXPORT int                  pfCylContainsPt(const pfCylinder* _cyl, const PFVEC3 pt);
extern DLLEXPORT void                 pfOrthoXformCyl(pfCylinder* _cyl, const pfCylinder *cyl, const PFMATRIX m);
extern DLLEXPORT void                 pfCylAroundPts(pfCylinder* _cyl, const pfVec3 *pts, int npt);
extern DLLEXPORT void                 pfCylAroundSegs(pfCylinder* _cyl, const pfSeg **segs, int nseg);
extern DLLEXPORT void                 pfCylAroundSpheres(pfCylinder* _cyl, const pfSphere **sphs, int nsph);
extern DLLEXPORT void                 pfCylAroundBoxes(pfCylinder* _cyl, const pfBox **boxes, int nbox);
extern DLLEXPORT void                 pfCylExtendBySphere(pfCylinder* _cyl, const pfSphere *sph);
extern DLLEXPORT void                 pfCylExtendByCyl(pfCylinder* _cyl, const pfCylinder *cyl);
extern DLLEXPORT void                 pfCylExtendByBox(pfCylinder* _cyl, const PFVEC3 pt);
extern DLLEXPORT int                  pfCylIsectSeg(const pfCylinder* _cyl, const pfSeg *seg, float *d1, float *d2);
extern DLLEXPORT void                 pfFromCylinderdCyl(pfCylinder* _cyl, pfCylinderd *cyl);

#endif /* !PF_C_API */

#ifdef PF_C_API
/*---------------- pfCylinderd CAPI ------------------------------*/

extern DLLEXPORT void                 pfMakeEmptyCyld(pfCylinderd* _cyld);
extern DLLEXPORT int                  pfCyldContainsPt(const pfCylinderd* _cyld, const PFVEC3 pt);
extern DLLEXPORT int                  pfCyldContainsPtd(const pfCylinderd* _cyld, const PFVEC3D pt);
extern DLLEXPORT void                 pfOrthoXformCyld(pfCylinderd* _cyld, const pfCylinderd *cyl, const PFMATRIX4D m);
extern DLLEXPORT void                 pfCyldAroundPts(pfCylinderd* _cyld, const pfVec3 *pts, int npt);
extern DLLEXPORT void                 pfCyldAroundSegs(pfCylinderd* _cyld, const pfSeg **segs, int nseg);
extern DLLEXPORT void                 pfCyldAroundSegds(pfCylinderd* _cyld, const pfSegd **segs, int nseg);
extern DLLEXPORT void                 pfCyldAroundSpheres(pfCylinderd* _cyld, const pfSphere **sphs, int nsph);
extern DLLEXPORT void                 pfCyldAroundSpheresdd(pfCylinderd* _cyld, const pfSpheredd **sphs, int nsph);
extern DLLEXPORT void                 pfCyldAroundBoxes(pfCylinderd* _cyld, const pfBox **boxes, int nbox);
extern DLLEXPORT void                 pfCyldExtendBySphere(pfCylinderd* _cyld, const pfSphere *sph);
extern DLLEXPORT void                 pfCyldExtendBySpheredd(pfCylinderd* _cyld, const pfSpheredd *sph);
extern DLLEXPORT void                 pfCyldExtendByCyl(pfCylinderd* _cyld, const pfCylinder *cyl);
extern DLLEXPORT void                 pfCyldExtendByCyld(pfCylinderd* _cyld, const pfCylinderd *cyl);
extern DLLEXPORT void                 pfCyldExtendByBox(pfCylinderd* _cyld, const PFVEC3 pt);
extern DLLEXPORT void                 pfCyldExtendByBoxd(pfCylinderd* _cyld, const PFVEC3D pt);
extern DLLEXPORT int                  pfCyldIsectSeg(const pfCylinderd* _cyld, const pfSeg *seg, float *d1, float *d2);
extern DLLEXPORT int                  pfCyldIsectSegd(const pfCylinderd* _cyld, const pfSegd *seg, double *d1, double *d2);
extern DLLEXPORT void                 pfFromCylinderCyld(pfCylinderd* _cyld, pfCylinder *cyl);

#endif /* !PF_C_API */

#ifdef PF_C_API
/*---------------- pfBox CAPI ------------------------------*/

extern DLLEXPORT void                 pfMakeEmptyBox(pfBox* _box);
extern DLLEXPORT int                  pfBoxContainsPt(const pfBox* _box, const PFVEC3 pt);
extern DLLEXPORT int                  pfBoxContainsBox(pfBox* _box, const pfBox *_inbox);
extern DLLEXPORT int                  pfBoxContainsSphere(const pfBox* _box, const pfSphere *sphere);
extern DLLEXPORT void                 pfXformBox(pfBox* _box, const pfBox *box, const PFMATRIX xform);
extern DLLEXPORT void                 pfBoxAroundPts(pfBox* _box, const pfVec3 *pts, int npt);
extern DLLEXPORT void                 pfBoxAroundSpheres(pfBox* _box, const pfSphere **sphs, int nsph);
extern DLLEXPORT void                 pfBoxAroundBoxes(pfBox* _box, const pfBox **boxes, int nbox);
extern DLLEXPORT void                 pfBoxAroundCyls(pfBox* _box, const pfCylinder **cyls, int ncyl);
extern DLLEXPORT void                 pfBoxExtendByPt(pfBox* _box, const PFVEC3 pt);
extern DLLEXPORT void                 pfBoxExtendByBox(pfBox* _box, const pfBox *box);
extern DLLEXPORT int                  pfBoxIsectSeg(const pfBox* _box, const pfSeg *seg, float *d1, float *d2);

#endif /* !PF_C_API */

#if !PF_CPLUSPLUS_API /* Also in C++ header  EXPORT to CAPI */

struct DLLEXPORT _pfSeg {
    pfVec3	pos;
    pfVec3	dir;
    float	length;
};

struct DLLEXPORT _pfSegd {
    pfVec3d	pos;
    pfVec3d	dir;
    double	length;
};

struct DLLEXPORT _pfPlane {
    pfVec3	normal;
    float	offset;		/* pt dot normal = offset */
};

struct DLLEXPORT _pfSphere {
    pfVec3	center;
    float	radius;
};

struct DLLEXPORT _pfCylinder {
    pfVec3	center;
    float	radius;
    pfVec3	axis;
    float	halfLength;
};

struct DLLEXPORT _pfCylinderd {
    pfVec3d	center;
    double	radius;
    pfVec3d	axis;
    double	halfLength;
};

struct DLLEXPORT _pfBox {
    pfVec3	min;
    pfVec3	max;
};

#define PFIS_MAX_SEGS		32 /* maximum number of segments per request */

struct DLLEXPORT _pfSegSet
{
    int			mode;
    void*		userData;
    pfSeg		segs[PFIS_MAX_SEGS];
    unsigned int	activeMask;
    unsigned int	isectMask;
    void*		bound;
    int 		(*discFunc)(pfHit*);
};

struct DLLEXPORT _pfSegSetd
{
    int			mode;
    void*		userData;
    pfSegd		segs[PFIS_MAX_SEGS];
    unsigned int	activeMask;
    unsigned int	isectMask;
    void*		bound;
    int 		(*discFunc)(pfHit*);
};

#endif /* !PF_CPLUSPLUS_API */

#ifdef PF_C_API
/*---------------- pfPolytope CAPI ------------------------------*/

extern DLLEXPORT pfPolytope*          pfNewPtope(void *arena);
extern DLLEXPORT pfType*              pfGetPtopeClassType(void);
extern DLLEXPORT int                  pfGetPtopeNumFacets(const pfPolytope* ptp);
extern DLLEXPORT int                  pfPtopeFacet(pfPolytope* ptp, int _i, const pfPlane *_p);
extern DLLEXPORT int                  pfGetPtopeFacet(const pfPolytope* ptp, int _i, pfPlane *_p);
extern DLLEXPORT int                  pfRemovePtopeFacet(pfPolytope* ptp, int _i);
extern DLLEXPORT void                 pfOrthoXformPtope(pfPolytope* ptp, const pfPolytope *_src, const PFMATRIX _mat);
extern DLLEXPORT int                  pfPtopeContainsPt(const pfPolytope* ptp, const PFVEC3 pt);
extern DLLEXPORT int                  pfPtopeContainsSphere(const pfPolytope* ptp, const pfSphere *sphere);
extern DLLEXPORT int                  pfPtopeContainsBox(const pfPolytope* ptp, const pfBox *box);
extern DLLEXPORT int                  pfPtopeContainsCyl(const pfPolytope* ptp, const pfCylinder *cyl);
extern DLLEXPORT int                  pfPtopeContainsPtope(const pfPolytope* ptp, const pfPolytope *ptope);

#endif /* !PF_C_API */

#if !PF_CPLUSPLUS_API /* Also in C++ header  EXPORT to CAPI */
/* ----------------------- pfFrustum Tokens ----------------------- */

#define PFFRUST_SIMPLE		0
#define PFFRUST_ORTHOGONAL	1
#define PFFRUST_PERSPECTIVE	2

#define PFFRUST_CALC_NONE	0
#define PFFRUST_CALC_HORIZ	1
#define PFFRUST_CALC_VERT	2


#endif /* !PF_CPLUSPLUS_API */

#ifdef PF_C_API
/*---------------- pfFrustum CAPI ------------------------------*/

extern DLLEXPORT pfFrustum*           pfNewFrust(void *arena);
extern DLLEXPORT pfType*              pfGetFrustClassType(void);
extern DLLEXPORT int                  pfGetFrustType(const pfFrustum* fr);
extern DLLEXPORT void                 pfFrustAspect(pfFrustum* fr, int _which, float _widthHeightRatio);
extern DLLEXPORT float                pfGetFrustAspect(const pfFrustum* fr);
extern DLLEXPORT void                 pfGetFrustFOV(const pfFrustum* fr, float* _fovh, float* _fovv);
extern DLLEXPORT void                 pfFrustNearFar(pfFrustum* fr, float _nearDist, float _farDist);
extern DLLEXPORT void                 pfGetFrustNearFar(const pfFrustum* fr, float* _nearDist, float* _farDist);
extern DLLEXPORT void                 pfGetFrustNear(const pfFrustum* fr, PFVEC3 _ll, PFVEC3 _lr, PFVEC3 _ul, PFVEC3 _ur);
extern DLLEXPORT void                 pfGetFrustFar(const pfFrustum* fr, PFVEC3 _ll, PFVEC3 _lr, PFVEC3 _ul, PFVEC3 _ur);
extern DLLEXPORT void                 pfGetFrustPtope(const pfFrustum* fr, pfPolytope *dst);
extern DLLEXPORT void                 pfGetFrustGLProjMat(const pfFrustum* fr, PFMATRIX mat);
extern DLLEXPORT void                 pfGetFrustLeftRightBottomTop(const pfFrustum* fr, float *l, float *r, float *b, float *t);
extern DLLEXPORT int                  pfGetFrustEye(const pfFrustum* fr, PFVEC3 _eye);
extern DLLEXPORT void                 pfMakePerspFrust(pfFrustum* fr, float _left, float _right, float _bot, float _top);
extern DLLEXPORT void                 pfMakeOrthoFrust(pfFrustum* fr, float _left, float _right, float _bot, float _top);
extern DLLEXPORT void                 pfMakeSimpleFrust(pfFrustum* fr, float _fov);
extern DLLEXPORT void                 pfOrthoXformFrust(pfFrustum* fr, const pfFrustum* _fr2, const PFMATRIX _mat);
extern DLLEXPORT int                  pfRecommendNearFrustSphere(const pfFrustum* fr, const pfSphere *sph, PFVEC3 result_point, int use_cone);
extern DLLEXPORT int                  pfRecommendNearFrustPolygon(const pfFrustum* fr, int nVerts, pfVec3 convexPolygonVerts[], PFVEC3 recommended, pfVec2 convexPolygonTexCoords[], PFVEC2 recommendedTextureCoord);
extern DLLEXPORT int                  pfFrustContainsPt(const pfFrustum* fr, const PFVEC3 pt);
extern DLLEXPORT int                  pfFrustContainsSphere(const pfFrustum* fr, const pfSphere *sphere);
extern DLLEXPORT int                  pfFrustContainsBox(const pfFrustum* fr, const pfBox *box);
extern DLLEXPORT int                  pfFrustContainsCyl(const pfFrustum* fr, const pfCylinder *cyl);
extern DLLEXPORT void                 pfApplyFrust(const pfFrustum* fr);

#endif /* !PF_C_API */

#if !PF_CPLUSPLUS_API /* Also in C++ header  EXPORT to CAPI */

/*------------------------- Triangle Intersection ---------------------------*/

extern int DLLEXPORT pfTriIsectSeg(const PFVEC3 _pt1, const PFVEC3 _pt2, const PFVEC3 _pt3, const pfSeg* _seg, float* _d);

/*
 * Isection 
 */
#define PFBOX_CONTAINS_PT(_box, _pt) \
   ((_pt)[0] >= (_box)->min[0] && \
    (_pt)[1] >= (_box)->min[1] && \
    (_pt)[2] >= (_box)->min[2] && \
    (_pt)[0] <= (_box)->max[0] && \
    (_pt)[1] <= (_box)->max[1] && \
    (_pt)[2] <= (_box)->max[2])

#define PFHALF_SPACE_CONTAINS_PT(_pln, _pt) \
	(PFDOT_VEC3((_pt), (_pln)->normal) <= (_pln)->offset)

#define PFSPHERE_CONTAINS_PT(_sp, _pt) \
	(PFSQR_DISTANCE_PT3((_sp)->center, (_pt)) <= PF_SQUARE((_sp)->radius))

/*
 * Boxes
 */

#define PFBOX_EXTENDBY_BOX(_dst, _box) \
    ((_box)->min[0] < (_dst)->min[0] ? (_dst)->min[0] = (_box)->min[0] : 0, \
     (_box)->min[1] < (_dst)->min[1] ? (_dst)->min[1] = (_box)->min[1] : 0, \
     (_box)->min[2] < (_dst)->min[2] ? (_dst)->min[2] = (_box)->min[2] : 0, \
     (_box)->max[0] > (_dst)->max[0] ? (_dst)->max[0] = (_box)->max[0] : 0, \
     (_box)->max[1] > (_dst)->max[1] ? (_dst)->max[1] = (_box)->max[1] : 0, \
     (_box)->max[2] > (_dst)->max[2] ? (_dst)->max[2] = (_box)->max[2] : 0)

/* 
 * Always testing against both bounds is not redundant because
 * box may be "empty" in one or more dimensions.
 */
#define PFBOX_EXTENDBY_PT(_dst, _pt) \
    ((_pt)[0] < (_dst)->min[0] ? (_dst)->min[0] = (_pt)[0] : 0, \
     (_pt)[1] < (_dst)->min[1] ? (_dst)->min[1] = (_pt)[1] : 0, \
     (_pt)[2] < (_dst)->min[2] ? (_dst)->min[2] = (_pt)[2] : 0, \
     (_pt)[0] > (_dst)->max[0] ? (_dst)->max[0] = (_pt)[0] : 0, \
     (_pt)[1] > (_dst)->max[1] ? (_dst)->max[1] = (_pt)[1] : 0, \
     (_pt)[2] > (_dst)->max[2] ? (_dst)->max[2] = (_pt)[2] : 0)

#define PFNONEMPTY_BOX_EXTENDBY_PT(_dst, _pt) \
    (((_pt)[0] < (_dst)->min[0] ? (_dst)->min[0] = (_pt)[0] : \
      ((_pt)[0] > (_dst)->max[0] ? (_dst)->max[0] = (_pt)[0] : 0)), \
     ((_pt)[1] < (_dst)->min[1] ? (_dst)->min[1] = (_pt)[1] : \
      ((_pt)[1] > (_dst)->max[1] ? (_dst)->max[1] = (_pt)[1] : 0)), \
     ((_pt)[2] < (_dst)->min[2] ? (_dst)->min[2] = (_pt)[2] : \
      ((_pt)[2] > (_dst)->max[2] ? (_dst)->max[2] = (_pt)[2] : 0)))

#define PFORTHO_XFORM_SPHERE(_dst, _sph, _m) \
	((_dst)->center.xformPt((sph)->center, (_m)), \
	 (_dst)->radius = ((_sph)->radius * \
			   pfSqrt(PF_SQUARE((_m)[0][0]) + \
				  PF_SQUARE((_m)[0][1]) + \
				  PF_SQUARE((_m)[0][2]))))
	    
#endif /* !PF_CPLUSPLUS_API */
#endif /* PF_C_API */

/*
 * Convenience macros
 */
#define PF_SQUARE(_x) ((_x)*(_x))
#define PF_MIN2(a,b) ((a) < (b) ? (a) : (b))
#define PF_MAX2(a,b) ((a) > (b) ? (a) : (b))
#define PF_MIN3(a,b,c) ((a) < (b) ? PF_MIN2(a,c) : PF_MIN2(b,c))
#define PF_MAX3(a,b,c) ((a) > (b) ? PF_MAX2(a,c) : PF_MAX2(b,c))
#define PF_MIN4(a,b,c,d) ((a) < (b) ? PF_MIN3(a,c,d) : PF_MIN3(b,c,d))
#define PF_MAX4(a,b,c,d) ((a) > (b) ? PF_MAX3(a,c,d) : PF_MAX3(b,c,d))

#define PF_CLAMP(_x, _lo, _hi) \
	(((_x) < (_lo)) ? (_lo) : (_x) > (_hi) ? (_hi) : (_x))

/* 
 * PF_ABS is faster than calling fabsf
 * PF_ABSLT etc are faster than (PF_ABS(x1) < x2)
 */
#define PF_ABS(_x1) ((_x1 < 0) ? -(_x1) : (_x1))	
#define PF_ABSLT(_x1,_x2) ((_x1) < (_x2) && -(_x1) < (_x2))
#define PF_ABSGT(_x1,_x2) ((_x1) > (_x2) || -(_x1) > (_x2))
#define PF_ABSLE(_x1,_x2) ((_x1) <= (_x2) && -(_x1) <= (_x2))
#define PF_ABSGE(_x1,_x2) ((_x1) >= (_x2) || -(_x1) >= (_x2))

/* 
 * Speed oriented macros
 */
#if defined (__STDC__) || defined(WIN32)	  /* ANSI C knows about float constants */
#define PF_PI	    3.14159265358979323846f /* F for SP float */
#define PF_PI_D	    3.14159265358979323846  /* slower DP for more precision */

#define PF_DEG2RAD(x)   ((x)*PF_PI  /180.0f)
#define PF_DEG2RAD_D(x) ((x)*PF_PI_D/180.0)

#define PF_RAD2DEG(x)   ((x)*180.0f/PF_PI)
#define PF_RAD2DEG_D(x) ((x)*180.0 /PF_PI_D)

#define PF_HUGEVAL  3.40282347e+37f

 
/* macro for fast square roots */
/* thresholds chosen so it's no worse than pfSqrt() */
#define PF_SQRT1(_x) \
	(((_x) > 0.9996f && (_x) < 1.001f) ? \
	  0.5f + 0.5f*(_x) : \
	  pfSqrt(_x))

#define PF_SQRT1_D(_x) \
	(((_x) > 0.99999999f && (_x) < 1.0000001f) ? \
	  0.5 + 0.5*(_x) : \
	  sqrt(_x))

#define PF_1OVERSQRT1(_x) \
	(((_x) > 0.9996f && (_x) < 1.001f) ? \
	  1.5f - 0.5f*(_x) : \
	  1.0f/pfSqrt(_x))

#else /* __STDC__ */ /* CCKR C doesn't understand float constants -> slow fp */

#define PF_PI	    3.14159265358979323846
#define PF_HUGEVAL  3.40282347e+37
#define PF_DEG2RAD(x) ((x)*PF_PI/180.0)
#define PF_RAD2DEG(x) ((x)*180.0/PF_PI)

/* macro for fast square roots of things probably near one */
/* thresholds chosen so it's no worse than pfSqrt() */
#define PF_SQRT1(_x) \
	(((_x) > 0.9996 && (_x) < 1.001) ? \
	 0.5 + 0.5 * (_x) : \
	 pfSqrt(_x))

#define PF_1OVERSQRT1(_x) \
	(((_x) > 0.9996 && (_x) < 1.001) ? \
	  1.5 - 0.5*(_x) : \
	  1.0/pfSqrt(_x))

#endif /* __STDC__ */ 

#ifdef __cplusplus
} /* extern "C" */
#endif

    /*****************************************************
     *  Math Wrappers
     *****************************************************/

/* Math macros --- C_API only*/
#if PF_C_API
    #define pfSinCos(_arg, _sPtr, _cPtr) \
	( ( (*(_cPtr) = PF_DEG2RAD(_arg)), \
	   (*(_sPtr) = sinf(*(_cPtr))), \
           (*(_cPtr) = cosf(*(_cPtr))) ) )

    #define pfSinCosd(_arg, _sPtr, _cPtr) \
	( ( (*(_cPtr) = PF_DEG2RAD_D(_arg)), \
	   (*(_sPtr) = sin(*(_cPtr))), \
           (*(_cPtr) = cos(*(_cPtr))) ) )

    #define pfTan(_arg) (tanf(PF_DEG2RAD(_arg)))	
    #define pfTand(_arg) (tan(PF_DEG2RAD_D(_arg)))
    #define pfArcTan2(_y, _x) (PF_RAD2DEG(atan2f(_y , _x)))
    #define pfArcTan2d(_y, _x) (PF_RAD2DEG_D(atan2(_y , _x)))
    #define pfArcSin(_arg) (PF_RAD2DEG(asinf(_arg)))
    #define pfArcSind(_arg) (PF_RAD2DEG_D(asin(_arg)))
    #define pfArcCos(_arg) (PF_RAD2DEG(acosf(_arg)))
    #define pfArcCosd(_arg) (PF_RAD2DEG_D(acos(_arg)))
    #define pfSqrt(_arg) (sqrtf(_arg))

#else /* ASSERT: PFCPLUSPLUS_API */

#define INLINE __declspec(dllexport) inline

    INLINE void pfSinCos(float arg, float* s, float* c)
	{ *s = sinf(PF_DEG2RAD(arg)); *c = cosf(PF_DEG2RAD(arg)); }
    INLINE void pfSinCosd(double arg, double* s, double* c)
	{ *s = sin(PF_DEG2RAD_D(arg)); *c = cos(PF_DEG2RAD_D(arg)); }
    INLINE float pfTan(float arg)
	{ return tanf(PF_DEG2RAD(arg)); }
    INLINE double pfTand(double arg)
	{ return tan(PF_DEG2RAD_D(arg)); }
    INLINE float pfArcTan2(float y, float x)
	{
	    // work around bug #338208 -- atan2f(0,-0) == NaN
	    if (y == 0.0f && x == 0.0f) return 0.0f;
	    return PF_RAD2DEG(atan2f(y, x));
	}
    INLINE double pfArcTan2d(double y, double x)
	{ return PF_RAD2DEG_D(atan2(y, x)); }
    INLINE float pfArcSin(float arg)
	{ return PF_RAD2DEG(asinf(arg)); }
    INLINE double pfArcSind(double arg)
	{ return PF_RAD2DEG_D(asin(arg)); }
    INLINE float pfArcCos(float arg)
	{ return PF_RAD2DEG(acosf(arg)); }
    INLINE double pfArcCosd(double arg)
	{ return PF_RAD2DEG_D(acos(arg)); }
    INLINE float pfSqrt(float a)
	{ return(sqrtf(a)); }
	
#endif /* PF_C_API */

#endif /* __PR_MATH_H__ */
