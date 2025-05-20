//
//
// Copyright 1995, Silicon Graphics, Inc.
// ALL RIGHTS RESERVED
//
// UNPUBLISHED -- Rights reserved under the copyright laws of the United
// States.   Use of a copyright notice is precautionary only and does not
// imply publication or disclosure.
//
// U.S. GOVERNMENT RESTRICTED RIGHTS LEGEND:
// Use, duplication or disclosure by the Government is subject to restrictions
// as set forth in FAR 52.227.19(c)(2) or subparagraph (c)(1)(ii) of the Rights
// in Technical Data and Computer Software clause at DFARS 252.227-7013 and/or
// in similar or successor clauses in the FAR, or the DOD or NASA FAR
// Supplement.  Contractor/manufacturer is Silicon Graphics, Inc.,
// 2011 N. Shoreline Blvd. Mountain View, CA 94039-7311.
//
// THE CONTENT OF THIS WORK CONTAINS CONFIDENTIAL AND PROPRIETARY
// INFORMATION OF SILICON GRAPHICS, INC. ANY DUPLICATION, MODIFICATION,
// DISTRIBUTION, OR DISCLOSURE IN ANY FORM, IN WHOLE, OR IN PART, IS STRICTLY
// PROHIBITED WITHOUT THE PRIOR EXPRESS WRITTEN PERMISSION OF SILICON
// GRAPHICS, INC.
//
// pfLinMath.h
//
// $Revision: 1.62 $
// $Date: 2002/12/09 11:05:17 $
//
//

#ifndef __PF_LINMATH_H__
#define __PF_LINMATH_H__

#ifndef PF_CPLUSPLUS_API
#define PF_CPLUSPLUS_API 1
#else
#if !PF_CPLUSPLUS_API
#error "Cannot include C++ API header with PF_CPLUSPLUS_API disabled."
#endif
#endif

#include <Performer/pr/pfObject.h>
#include <Performer/pr/pfStruct.h>

struct pfMatrix;
struct pfMatrix4d;
struct pfQuat;
struct pfQuatd;
class DLLEXPORT pfHit;

//CAPI:struct
struct DLLEXPORT pfVec2
{
    PFSTRUCT_DECLARE

public:
    float vec[2];

public:
    // constructors and destructors
    //CAPI:private
    pfVec2(float _x, float _y) { set(_x, _y); }
    pfVec2() {};

public:
    // sets and gets
    //CAPI:arrayclass
    //CAPI:verb SetVec2
    void set(float _x, float _y) { 
	vec[0] = _x; 
	vec[1] = _y;
    }

public:
    // other functions
    //CAPI:verb
    void copy(const pfVec2&  _v) { *this = _v; }
    int equal(const pfVec2&  _v) const { 
	return (vec[0] == _v[0] && 
		vec[1] == _v[1]);
    }
    int almostEqual(const pfVec2& _v, float _tol) const;

    void negate(const pfVec2& _v) { 
	vec[0] = -_v[0];
	vec[1] = -_v[1]; 
    }

    float dot(const pfVec2&  _v) const {
	return (vec[0] * _v[0] + 
		vec[1] * _v[1]);
    }

    void add(const pfVec2& _v1, const pfVec2& _v2) { 
	vec[0] = _v1[0] + _v2[0]; 
	vec[1] = _v1[1] + _v2[1]; 
    }

    void sub(const pfVec2& _v1, const pfVec2& _v2) { 
	vec[0] = _v1[0] - _v2[0]; 
	vec[1] = _v1[1] - _v2[1]; 
    }

    void scale(float _s, const pfVec2& _v) { 
	vec[0] = _s * _v[0]; 
	vec[1] = _s * _v[1]; 
    }

    void addScaled(const pfVec2& _v1, float _s, const pfVec2& _v2) { 
	vec[0] = _v1[0] + _s * _v2[0]; 
	vec[1] = _v1[1] + _s * _v2[1]; 
    }

    void combine(float _a, const pfVec2& _v1, float _b, const pfVec2& _v2) { 
	vec[0] = _a * _v1[0] + _b * _v2[0]; 
	vec[1] = _a * _v1[1] + _b * _v2[1]; 
    }

    //CAPI:verb SqrDistancePt2
    float sqrDistance(const pfVec2& _v) const { 
	return (PF_SQUARE(vec[0] - _v[0]) +
		PF_SQUARE(vec[1] - _v[1]));
    }

    float normalize();
    float length() const;
    //CAPI:verb DistancePt2
    float distance(const pfVec2& _v) const;

public:
    // Operators
    float&  operator [](int i) { return vec[i]; }
    const float&  operator [](int i) const { return vec[i]; }

    int operator ==(const pfVec2& _v) const { return equal(_v); }
    int operator !=(const pfVec2& _v) const { return !equal(_v); }

public:
    // pfVec2 operators (N.B. return by value can be slow)

    pfVec2 operator -() const {
        return pfVec2(-vec[0], -vec[1]);
    }

    pfVec2 operator +(const pfVec2& _v) const {
        return pfVec2(vec[0]+_v[0], vec[1]+_v[1]);
    }
    
    pfVec2 operator -(const pfVec2& _v) const {
        return pfVec2(vec[0]-_v[0], vec[1]-_v[1]);
    }

    friend inline pfVec2 operator *(float _s, const pfVec2&);
    friend inline pfVec2 operator *(const pfVec2& _v, float _s);
    friend inline pfVec2 operator /(const pfVec2& _v, float _s);

public:
    // Assignment Operators
    pfVec2&  operator =(const pfVec2& _v) {
        vec[0] = _v[0]; 
	vec[1] = _v[1];
	return *this;
    }

    pfVec2& operator *=(float _s) {
        vec[0] *= _s; 
	vec[1] *= _s;
	return *this;
    }
    
    pfVec2& operator /=(float _s) {
	_s = 1.0f/_s;
        return *this *= _s;
    }
    
    pfVec2& operator +=(const pfVec2& _v) {
        vec[0] += _v[0];
	vec[1] += _v[1];
	return *this;
    }

    pfVec2& operator -=(const pfVec2& _v) {
        vec[0] -= _v[0];
	vec[1] -= _v[1];
	return *this;
    }
};


inline pfVec2 operator *(float _s, const pfVec2& _v) {
    return pfVec2(_v[0] * _s, _v[1] * _s);
}

inline pfVec2 operator *(const pfVec2& _v, float _s) {
    return pfVec2(_v[0] * _s, _v[1] * _s);
}

inline pfVec2 operator /(const pfVec2& _v, float _s) {
    _s = 1.0f/_s;
    return pfVec2(_v[0]*_s, _v[1]*_s);
}

//CAPI:struct
struct DLLEXPORT pfVec3
{
    PFSTRUCT_DECLARE

public:
    float vec[3];

public:
    // constructors and destructors
    //CAPI:private
    pfVec3(float _x, float _y, float _z) { set(_x, _y, _z); }
    pfVec3() {};

public:
    // sets and gets
    //CAPI:arrayclass
    //CAPI:verb SetVec3
    void set(float _x, float _y, float _z) {
	vec[0] = _x;
	vec[1] = _y;
	vec[2] = _z; 
    }

public:
    // other functions
    //CAPI:verb
    void copy(const pfVec3&  _v) { *this = _v; }
    int equal(const pfVec3&  _v) const { 
	return (vec[0] == _v[0] && 
		vec[1] == _v[1] &&
		vec[2] == _v[2]);
    }
    int almostEqual(const pfVec3& _v, float _tol) const;

    void negate(const pfVec3& _v) { 
	vec[0] = -_v[0];
	vec[1] = -_v[1]; 
	vec[2] = -_v[2]; 
    }

    float dot(const pfVec3&  _v) const {
	return (vec[0] * _v[0] + 
		vec[1] * _v[1] +
		vec[2] * _v[2]);
    }

    void add(const pfVec3& _v1, const pfVec3& _v2) { 
	vec[0] = _v1[0] + _v2[0]; 
	vec[1] = _v1[1] + _v2[1]; 
	vec[2] = _v1[2] + _v2[2]; 
    }

    void sub(const pfVec3& _v1, const pfVec3& _v2) { 
	vec[0] = _v1[0] - _v2[0]; 
	vec[1] = _v1[1] - _v2[1]; 
	vec[2] = _v1[2] - _v2[2]; 
    }

    void scale(float _s, const pfVec3& _v) { 
	vec[0] = _s * _v[0]; 
	vec[1] = _s * _v[1]; 
	vec[2] = _s * _v[2]; 
    }

    void addScaled(const pfVec3& _v1, float _s, const pfVec3& _v2) { 
	vec[0] = _v1[0] + _s * _v2[0]; 
	vec[1] = _v1[1] + _s * _v2[1]; 
	vec[2] = _v1[2] + _s * _v2[2]; 
    }

    void combine(float _a, const pfVec3& _v1, float _b, const pfVec3& _v2) { 
	vec[0] = _a * _v1[0] + _b * _v2[0]; 
	vec[1] = _a * _v1[1] + _b * _v2[1]; 
	vec[2] = _a * _v1[2] + _b * _v2[2]; 
    }

    //CAPI:verb SqrDistancePt3
    float sqrDistance(const pfVec3& _v) const { 
	return (PF_SQUARE(vec[0] - _v[0]) +
		PF_SQUARE(vec[1] - _v[1]) +
		PF_SQUARE(vec[2] - _v[2]));
    }

    float normalize();
    float length() const;
    //CAPI:verb DistancePt3
    float distance(const pfVec3& _v) const;
    void  cross(const pfVec3&  _v1, const pfVec3&  _v2);

    //CAPI:verb XformVec3
    void xformVec(const pfVec3& _v, const pfMatrix& _m);

    //CAPI:verb XformPt3
    void xformPt(const pfVec3& _v, const pfMatrix& _m);

    //CAPI:verb FullXformPt3
    void fullXformPt(const pfVec3& _v, const pfMatrix& _m);

    //CAPI:verb FullXformPt3w
    float fullXformPtw(const pfVec3& _v, const pfMatrix& _m);

public:
    // Operators
    float&  operator [](int i) { return vec[i]; }

    const float&  operator [](int i) const { return vec[i]; }

    int operator ==(const pfVec3& _v) const {
        return vec[0] == _v[0] && vec[1] == _v[1] && vec[2] == _v[2];
    }
    int operator !=(const pfVec3& _v) const {
        return !(*this == _v);
    }

public:
    // pfVec3 operators (N.B. return by value can be slow)

    pfVec3 operator -() const {
        return pfVec3(-vec[0], -vec[1], -vec[2]);
    }

    pfVec3 operator +(const pfVec3& _v) const {
        return pfVec3(vec[0]+_v[0], vec[1]+_v[1], vec[2]+_v[2]);
    }

    pfVec3 operator -(const pfVec3& _v) const {
        return pfVec3(vec[0]-_v[0], vec[1]-_v[1], vec[2]-_v[2]);
    }

    friend inline pfVec3 operator *(float _s, const pfVec3&);
    friend inline pfVec3 operator *(const pfVec3& _v, float _s);
    friend inline pfVec3 operator /(const pfVec3& _v, float _s);
    friend inline pfVec3 operator *(const pfVec3& _v, const pfMatrix& _m);
    
public:
    // Assignment Operators
    pfVec3&  operator =(const pfVec3& _v) {
        vec[0] = _v[0]; 
	vec[1] = _v[1];
	vec[2] = _v[2]; 
	return *this;
    }

    pfVec3& operator *=(float _s) {
        vec[0] *= _s; 
	vec[1] *= _s; 
	vec[2] *= _s; 
	return *this;
    }
    
    pfVec3& operator /=(float _s) {
	_s = 1.0/_s; 
	return *this *= _s;
    }
    
    pfVec3& operator +=(const pfVec3& _v) {
        vec[0] += _v[0]; 
	vec[1] += _v[1]; 
	vec[2] += _v[2];
	return *this;
    }

    pfVec3& operator -=(const pfVec3& _v) {
        vec[0] -= _v[0]; 
	vec[1] -= _v[1]; 
	vec[2] -= _v[2];
	return *this;
    }
};


inline pfVec3 operator *(float _s, const pfVec3& _v) {
    return pfVec3(_v[0]*_s, _v[1]*_s, _v[2]*_s);
}

inline pfVec3 operator *(const pfVec3& _v, float _s) {
    return pfVec3(_v[0]*_s, _v[1]*_s, _v[2]*_s);
}

inline pfVec3 operator /(const pfVec3& _v, float _s) {
    _s = 1.0f/_s;
    return pfVec3(_v[0]*_s, _v[1]*_s, _v[2]*_s);
}

inline pfVec3 operator *(const pfVec3& _v, const pfMatrix&  _m) {
    // transform as point (w=1), assuming affine transformation
    // i.e. does not use slower dst.xformFullPt().
    pfVec3 dst; dst.xformPt(_v, _m); return dst;
}

//CAPI:struct
struct DLLEXPORT pfVec4
{
    PFSTRUCT_DECLARE

public:
    float vec[4];

public:
    // constructors and destructors
    //CAPI:arrayclass
    //CAPI:private
    pfVec4(float _x, float _y, float _z, float _w) { set(_x, _y, _z, _w); }
    pfVec4() {};

public:
    //CAPI:verb SetVec4
    void set(float _x, float _y, float _z, float _w) {
	vec[0] = _x;
	vec[1] = _y;
	vec[2] = _z; 
	vec[3] = _w; 
    }

public:
    // Other functions
    //CAPI:verb
    void copy(const pfVec4&  _v) { *this = _v; }
    int equal(const pfVec4&  _v) const { 
	return (vec[0] == _v[0] && 
		vec[1] == _v[1] &&
		vec[2] == _v[2] &&
		vec[3] == _v[3]);
    }
    int almostEqual(const pfVec4& _v, float _tol) const;

    void negate(const pfVec4& _v) { 
	vec[0] = -_v[0];
	vec[1] = -_v[1]; 
	vec[2] = -_v[2]; 
	vec[3] = -_v[3]; 
    }

    float dot(const pfVec4&  _v) const {
	return (vec[0] * _v[0] + 
		vec[1] * _v[1] +
		vec[2] * _v[2] +
		vec[3] * _v[3]);
    }

    void add(const pfVec4& _v1, const pfVec4& _v2) { 
	vec[0] = _v1[0] + _v2[0]; 
	vec[1] = _v1[1] + _v2[1]; 
	vec[2] = _v1[2] + _v2[2]; 
	vec[3] = _v1[3] + _v2[3]; 
    }

    void sub(const pfVec4& _v1, const pfVec4& _v2) { 
	vec[0] = _v1[0] - _v2[0]; 
	vec[1] = _v1[1] - _v2[1]; 
	vec[2] = _v1[2] - _v2[2]; 
	vec[3] = _v1[3] - _v2[3]; 
    }

    void scale(float _s, const pfVec4& _v) { 
	vec[0] = _s * _v[0]; 
	vec[1] = _s * _v[1]; 
	vec[2] = _s * _v[2]; 
	vec[3] = _s * _v[3]; 
    }

    void addScaled(const pfVec4& _v1, float _s, const pfVec4& _v2) { 
	vec[0] = _v1[0] + _s * _v2[0]; 
	vec[1] = _v1[1] + _s * _v2[1]; 
	vec[2] = _v1[2] + _s * _v2[2]; 
	vec[3] = _v1[3] + _s * _v2[3]; 
    }

    void combine(float _a, const pfVec4& _v1, float _b, const pfVec4& _v2) { 
	vec[0] = _a * _v1[0] + _b * _v2[0]; 
	vec[1] = _a * _v1[1] + _b * _v2[1]; 
	vec[2] = _a * _v1[2] + _b * _v2[2]; 
	vec[3] = _a * _v1[3] + _b * _v2[3]; 
    }

    //CAPI:verb SqrDistancePt4
    float sqrDistance(const pfVec4& _v) const { 
	return (PF_SQUARE(vec[0] - _v[0]) +
		PF_SQUARE(vec[1] - _v[1]) +
		PF_SQUARE(vec[2] - _v[2]) +
		PF_SQUARE(vec[3] - _v[3]));
    }

    float normalize();
    float length() const;
    //CAPI:verb DistancePt4
    float distance(const pfVec4& _v) const;

    void xform(const pfVec4& v, const pfMatrix& m);

public:
    // Operators
    float&  operator [](int i) { return vec[i]; }

    const float&  operator [](int i) const { return vec[i]; }
    
    int operator ==(const pfVec4& _v) const {
        return (vec[0] == _v[0] && vec[1] == _v[1] &&
		vec[2] == _v[2] && vec[3] == _v[3]);
    }
    int operator !=(const pfVec4& _v) const {
        return !(*this == _v);
    }
    
public:
    // pfVec4 operators (N.B. return by value can be slow)

    pfVec4 operator -() const {
        return pfVec4(-vec[0], -vec[1], -vec[2], -vec[3]);
    }

    pfVec4 operator +(const pfVec4& _v) const {
        return pfVec4(vec[0]+_v[0], vec[1]+_v[1],
		      vec[2]+_v[2], vec[3]+_v[3]);
    }
    
    pfVec4 operator -(const pfVec4& _v) const {
        return pfVec4(vec[0]-_v[0], vec[1]-_v[1],
		      vec[2]-_v[2], vec[3]-_v[3]);
    }

    friend inline pfVec4 operator *(float _s, const pfVec4&);
    friend inline pfVec4 operator *(const pfVec4& _v, float _s);
    friend inline pfVec4 operator /(const pfVec4& _v, float _s);
    friend inline pfVec4 operator *(const pfVec4& _v, const pfMatrix& _m);
    
public:
    // Assignment Operators
    pfVec4&  operator =(const pfVec4& _v) {
        vec[0] = _v[0]; vec[1] = _v[1]; 
	vec[2] = _v[2]; vec[3] = _v[3]; 
	return *this;
    }

    pfVec4& operator *=(float _s) {
        vec[0] *= _s; vec[1] *= _s; 
	vec[2] *= _s; vec[3] *= _s; 
	return *this;
    }
    
    pfVec4& operator /=(float _s) {
	_s = 1.0f/_s; return *this *= _s;
    }
    
    pfVec4& operator +=(const pfVec4& _v) {
        vec[0] += _v[0]; vec[1] += _v[1]; 
	vec[2] += _v[2]; vec[3] += _v[3];
	return *this;
    }

    pfVec4& operator -=(const pfVec4& _v) {
        vec[0] -= _v[0]; vec[1] -= _v[1]; 
	vec[2] -= _v[2]; vec[3] -= _v[3];
	return *this;
    }
};


inline pfVec4 operator *(const pfVec4& _v, const pfMatrix&  _m) {
    pfVec4 dst; dst.xform(_v, _m); return dst;
}

inline pfVec4 operator *(float _s, const pfVec4& _v) {
    return pfVec4(_v[0]*_s, _v[1]*_s, _v[2]*_s, _v[3]*_s);
}

inline pfVec4 operator *(const pfVec4& _v, float _s) {
    return pfVec4(_v[0]*_s, _v[1]*_s, _v[2]*_s, _v[3]*_s);
}

inline pfVec4 operator /(const pfVec4& _v, float _s) {
    _s = 1.0f/_s;
    return pfVec4(_v[0]*_s, _v[1]*_s, _v[2]*_s, _v[3]*_s);
}

//CAPI:struct
struct DLLEXPORT pfMatrix
{
    PFSTRUCT_DECLARE

public:
    float mat[4][4];

public:
    // constructors and destructors
    //CAPI:basename Mat
    //CAPI:arrayclass
    //CAPI:private
    pfMatrix() {};
    pfMatrix(float a00, float a01, float a02, float a03,
             float a10, float a11, float a12, float a13,
             float a20, float a21, float a22, float a23,
             float a30, float a31, float a32, float a33);
    int print(uint _travMode, uint _verbose, char *_prefix, FILE *_file);
#if 0
#ifdef __linux__
    /* don't quite understand why this isn't defined in the IRIX version */
    pfMatrix(float m[4][4]) {memcpy (mat, m, sizeof(float) * 16);};
#endif
#endif

public:
    // sets and gets
    //CAPI:verb SetMat
    void 	set(float *m) {
	memcpy(mat, m, sizeof(float) * 16);
    }
    //CAPI:verb GetMatType
    int		getMatType() const;
    //CAPI:verb SetMatRowVec3
    void	setRow(int _r, const pfVec3&  _v);
    //CAPI:verb SetMatRow
    void	setRow(int _r, float _x, float _y, float _z, float _w) {
	mat[_r][0] = _x; mat[_r][1] = _y; mat[_r][2] = _z; mat[_r][3] = _w;
    }
    //CAPI:verb GetMatRowVec3
    void	getRow(int _r, pfVec3&  _dst) const;
    //CAPI:verb GetMatRow
    void	getRow(int _r, float *_x, float *_y, float *_z, float *_w) const {
	*_x = mat[_r][0]; *_y = mat[_r][1]; *_z = mat[_r][2]; *_w = mat[_r][3];
    }
    //CAPI:verb SetMatColVec3
    void	setCol(int _c, const pfVec3&  _v);
    //CAPI:verb SetMatCol
    void	setCol(int _c, float _x, float _y, float _z, float _w) {
	mat[0][_c] = _x; mat[1][_c] = _y; mat[2][_c] = _z; mat[3][_c] = _w;
    }
    //CAPI:verb GetMatColVec3
    void	getCol(int _c, pfVec3& _dst) const;
    //CAPI:verb GetMatCol
    void	getCol(int _c, float *_x, float *_y, float *_z, float *_w) const {
	*_x = mat[0][_c]; *_y = mat[1][_c]; *_z = mat[2][_c]; *_w = mat[3][_c];
    }
    //CAPI:verb GetOrthoMatCoord
    void	getOrthoCoord(pfCoord* _dst) const;

public:
    // Other functions
    //CAPI:verb
    void	makeIdent();
    void	makeEuler(float _hdeg, float _pdeg, float _rdeg);
    void	makeRot(float _degrees, float _x, float _y, float _z);
    void	makeTrans(float _x, float _y, float _z);
    void	makeScale(float _x, float _y, float _z);
    void	makeVecRotVec(const pfVec3&  _v1, const pfVec3&  _v2);
    void	makeCoord(const pfCoord* _c);

    // convert quaternion to and from rotation matrix representation 
    //CAPI:verb GetOrthoMatQuat
    void	getOrthoQuat(pfQuat& dst) const;
    void	makeQuat(const pfQuat& q);

    void	copy(const pfMatrix&  _v) { *this = _v; }
    int		equal(const pfMatrix&  _m) const  {
        return (((mat)[0][0] == (_m)[0][0]) &&
		((mat)[0][1] == (_m)[0][1]) &&
		((mat)[0][2] == (_m)[0][2]) &&
		((mat)[0][3] == (_m)[0][3]) &&
		((mat)[1][0] == (_m)[1][0]) &&
		((mat)[1][1] == (_m)[1][1]) &&
		((mat)[1][2] == (_m)[1][2]) &&
		((mat)[1][3] == (_m)[1][3]) &&
		((mat)[2][0] == (_m)[2][0]) &&
		((mat)[2][1] == (_m)[2][1]) &&
		((mat)[2][2] == (_m)[2][2]) &&
		((mat)[2][3] == (_m)[2][3]) &&
		((mat)[3][0] == (_m)[3][0]) &&
		((mat)[3][1] == (_m)[3][1]) &&
		((mat)[3][2] == (_m)[3][2]) &&
		((mat)[3][3] == (_m)[3][3]));
    }

    int almostEqual(const pfMatrix&  _m2, float _tol) const;

public:
    // Mathematical operations on full 4X4
    //CAPI:verb
    void transpose(pfMatrix&  _m);
    void mult(const pfMatrix& _m1, const pfMatrix &m2);
    void add(const pfMatrix& _m1, const pfMatrix &m2);
    void sub(const pfMatrix& _m1, const pfMatrix &m2);

    // scale() multiplies full 4X4 by scalar, not a 3D scale xform
    //CAPI:header /* pfScaleMat() multiplies full 4X4 by scalar, not a 3D scale xform */
    void scale(float _s, const pfMatrix &_m);

    void postMult(const pfMatrix&  _m);
    void preMult(const pfMatrix&  _m);

    int invertFull(pfMatrix&  _m);
    void invertAff(const pfMatrix&  _m);
    void invertOrtho(const pfMatrix&  _m);
    void invertOrthoN(pfMatrix&  _m);
    void invertIdent(const pfMatrix&  _m);

public:
    //CAPI:verb
    // Transformation functions
    void preTrans(float _x, float _y, float _z, pfMatrix&  _m);
    void postTrans(const pfMatrix&  _m, float _x, float _y, float _z);

    void preRot(float _degrees, float _x, float _y, float _z, pfMatrix&  _m);
    void postRot(const pfMatrix&  _m, float _degrees, float _x, float _y, float _z);
    void preScale(float _xs, float _ys, float _zs, pfMatrix&  _m);
    void postScale(const pfMatrix&  _m, float _xs, float _ys, float _zs);

public:
    // Operators
    float*       operator [](int i)        { return &mat[i][0]; }
    const float* operator [](int i) const  { return &mat[i][0]; }
    
    int operator == (const pfMatrix&  _m) const {
	return this->equal(_m);
    }

    int operator != (const pfMatrix&  _m) const {
	return !this->equal(_m);
    }

public:
    // pfMatrix operators (N.B. return by value can be quite slow)
    pfMatrix operator *(const pfMatrix&  _m) const {
	pfMatrix dst; dst.mult(*this, _m); return dst;
    }

    pfMatrix operator +(const pfMatrix&  _m) const {
	pfMatrix dst; dst.add(*this, _m); return dst;
    }
    pfMatrix operator -(const pfMatrix&  _m) const {
	pfMatrix dst; dst.sub(*this, _m); return dst;
    }

    friend inline pfMatrix operator *(float _s, const pfMatrix&);
    friend inline pfMatrix operator *(const pfMatrix& _v, float _s);
    friend inline pfMatrix operator /(const pfMatrix& _v, float _s);

public:
    // Assignment operators
    pfMatrix&  operator =(const pfMatrix&  _m)
    {
        ((mat)[0][0] = (_m)[0][0]);
        ((mat)[0][1] = (_m)[0][1]);
        ((mat)[0][2] = (_m)[0][2]);
        ((mat)[0][3] = (_m)[0][3]);
        ((mat)[1][0] = (_m)[1][0]);
        ((mat)[1][1] = (_m)[1][1]);
        ((mat)[1][2] = (_m)[1][2]);
        ((mat)[1][3] = (_m)[1][3]);
        ((mat)[2][0] = (_m)[2][0]);
        ((mat)[2][1] = (_m)[2][1]);
        ((mat)[2][2] = (_m)[2][2]);
        ((mat)[2][3] = (_m)[2][3]);
        ((mat)[3][0] = (_m)[3][0]);
        ((mat)[3][1] = (_m)[3][1]);
        ((mat)[3][2] = (_m)[3][2]);
        ((mat)[3][3] = (_m)[3][3]);
        return *this;
    }

    pfMatrix&  operator *=(const pfMatrix&  _m)  {
	this->postMult(_m); return *this; 
    }

    pfMatrix&  operator *=(float _s);
    pfMatrix&  operator /=(float _s);
    pfMatrix&  operator +=(const pfMatrix&  _m);
    pfMatrix&  operator -=(const pfMatrix&  _m);
};

inline pfMatrix operator *(float _s, const pfMatrix& _m) {
    pfMatrix dst; dst.scale(_s, _m); return dst;
}

inline pfMatrix operator *(const pfMatrix& _m, float _s) {
    pfMatrix dst; dst.scale(_s, _m); return dst;
}

inline pfMatrix operator /(const pfMatrix& _m, float _s) {
    pfMatrix dst; dst.scale(1.0f/_s, _m); return dst;
}

extern "C" {     // EXPORT to CAPI
/*
 * pfQuat derives from pfVec4.
 * pfVec4 routines can also operate on pfQuats.
 */
} // extern "C"

//CAPI:struct
struct DLLEXPORT pfQuat : public pfVec4
{
    PFSTRUCT_DECLARE

public:
    // constructors and destructors
    //CAPI:arrayclass
    //CAPI:private
    pfQuat(float _x, float _y, float _z, float _w) { set(_x, _y, _z, _w); }
    pfQuat() {};

    //CAPI:private
    pfQuat(const float v[4]) { set(v[0], v[1], v[2], v[3]); }
    //CAPI:private
    pfQuat(const pfMatrix &m) { makeRot(m); }
    //CAPI:private
    pfQuat(const pfVec3 &axis, float _angle) { makeRot(_angle, axis[0], axis[1], axis[2]); }
    //CAPI:private
    pfQuat(const pfVec3 &rotateFrom, const pfVec3 rotateTo) { makeVecRotVec(rotateFrom, rotateTo); }


public:
    void getRot(float *_angle, float *_x, float *_y, float *_z) const;
    //CAPI:verb
    void makeRot(float _angle, float _x, float _y, float _z);

    void getRot(pfMatrix &m);
    //CAPI:verb MakeMatRotQuat
    void makeRot(const pfMatrix &m);
    //CAPI:verb
    void makeVecRotVec(const pfVec3 &rotateFrom, const pfVec3 rotateTo);

public:
    // Other functions
    //CAPI:verb
    void conj(const pfQuat& _v) { 
	vec[0] = -_v[0];
	vec[1] = -_v[1]; 
	vec[2] = -_v[2]; 
	vec[3] =  _v[3]; 
    }

    float length() const {
	return (vec[0]*vec[0]+vec[1]*vec[1]+vec[2]*vec[2]+vec[3]*vec[3]);	
    }
    
public:
    //CAPI:verb
    void mult(const pfQuat& q1, const pfQuat& q2);
    void div(const pfQuat& q1, const pfQuat& q2);
    void invert(const pfQuat& q1);
    void exp(const pfQuat& _q);
    void log(const pfQuat& _q);

    void slerp(float _t, const pfQuat& _q1, const pfQuat& _q2);
    void squad(float _t, const pfQuat& _q1, const pfQuat& _q2, const pfQuat& _a, const pfQuat& _b);

    //CAPI:noverb
    void meanTangent(const pfQuat& _q1, const pfQuat& _q2, const pfQuat& _q3);

public:
    // pfQuat operators (N.B. return by value can be slow)
    pfQuat operator *(const pfQuat&  _m) const {
	pfQuat dst; dst.mult(*this, _m); return dst;
    }

    pfQuat operator /(const pfQuat&  _m) const {
	pfQuat dst; dst.div(*this, _m); return dst;
    }

public:
    // Assignment operators
    pfQuat&  operator *=(const pfQuat& _q) {
	this->mult(*this, _q); return *this;
    }
    pfQuat&  operator /=(const pfQuat& _q) {
	this->div(*this, _q); return *this;
    }
};

//CAPI:struct
struct DLLEXPORT pfVec2d
{
    PFSTRUCT_DECLARE

public:
    double vec[2];

public:
    // constructors and destructors
    //CAPI:private
    pfVec2d(double _x, double _y) { set(_x, _y); }
    pfVec2d() {};

public:
    // sets and gets
    //CAPI:arrayclass
    //CAPI:verb SetVec2d
    void set(double _x, double _y) { 
	vec[0] = _x; 
	vec[1] = _y;
    }

public:
    // other functions
    //CAPI:verb
    void copy(const pfVec2d&  _v) { *this = _v; }
    int equal(const pfVec2d&  _v) const { 
	return (vec[0] == _v[0] && 
		vec[1] == _v[1]);
    }
    int almostEqual(const pfVec2d& _v, double _tol) const;

    void negate(const pfVec2d& _v) { 
	vec[0] = -_v[0];
	vec[1] = -_v[1]; 
    }

    double dot(const pfVec2d&  _v) const {
	return (vec[0] * _v[0] + 
		vec[1] * _v[1]);
    }

    void add(const pfVec2d& _v1, const pfVec2d& _v2) { 
	vec[0] = _v1[0] + _v2[0]; 
	vec[1] = _v1[1] + _v2[1]; 
    }

    void sub(const pfVec2d& _v1, const pfVec2d& _v2) { 
	vec[0] = _v1[0] - _v2[0]; 
	vec[1] = _v1[1] - _v2[1]; 
    }

    void scale(double _s, const pfVec2d& _v) { 
	vec[0] = _s * _v[0]; 
	vec[1] = _s * _v[1]; 
    }

    void addScaled(const pfVec2d& _v1, double _s, const pfVec2d& _v2) { 
	vec[0] = _v1[0] + _s * _v2[0]; 
	vec[1] = _v1[1] + _s * _v2[1]; 
    }

    void combine(double _a, const pfVec2d& _v1, double _b, const pfVec2d& _v2) { 
	vec[0] = _a * _v1[0] + _b * _v2[0]; 
	vec[1] = _a * _v1[1] + _b * _v2[1]; 
    }

    //CAPI:verb SqrDistancePt2d
    double sqrDistance(const pfVec2d& _v) const { 
	return (PF_SQUARE(vec[0] - _v[0]) +
		PF_SQUARE(vec[1] - _v[1]));
    }

    double normalize();
    double length() const;
    //CAPI:verb DistancePt2d
    double distance(const pfVec2d& _v) const;

public:
    // Operators
    double&  operator [](int i) { return vec[i]; }
    const double&  operator [](int i) const { return vec[i]; }

    int operator ==(const pfVec2d& _v) const { return equal(_v); }
    int operator !=(const pfVec2d& _v) const { return !equal(_v); }

public:
    // pfVec2d operators (N.B. return by value can be slow)

    pfVec2d operator -() const {
        return pfVec2d(-vec[0], -vec[1]);
    }

    pfVec2d operator +(const pfVec2d& _v) const {
        return pfVec2d(vec[0]+_v[0], vec[1]+_v[1]);
    }
    
    pfVec2d operator -(const pfVec2d& _v) const {
        return pfVec2d(vec[0]-_v[0], vec[1]-_v[1]);
    }

    friend inline pfVec2d operator *(double _s, const pfVec2d&);
    friend inline pfVec2d operator *(const pfVec2d& _v, double _s);
    friend inline pfVec2d operator /(const pfVec2d& _v, double _s);

public:
    // Assignment Operators
    pfVec2d&  operator =(const pfVec2d& _v) {
        vec[0] = _v[0]; 
	vec[1] = _v[1];
	return *this;
    }

    pfVec2d& operator *=(double _s) {
        vec[0] *= _s; 
	vec[1] *= _s;
	return *this;
    }
    
    pfVec2d& operator /=(double _s) {
	_s = 1.0f/_s;
        return *this *= _s;
    }
    
    pfVec2d& operator +=(const pfVec2d& _v) {
        vec[0] += _v[0];
	vec[1] += _v[1];
	return *this;
    }

    pfVec2d& operator -=(const pfVec2d& _v) {
        vec[0] -= _v[0];
	vec[1] -= _v[1];
	return *this;
    }
};


inline pfVec2d operator *(double _s, const pfVec2d& _v) {
    return pfVec2d(_v[0] * _s, _v[1] * _s);
}

inline pfVec2d operator *(const pfVec2d& _v, double _s) {
    return pfVec2d(_v[0] * _s, _v[1] * _s);
}

inline pfVec2d operator /(const pfVec2d& _v, double _s) {
    _s = 1.0f/_s;
    return pfVec2d(_v[0]*_s, _v[1]*_s);
}

//CAPI:struct
struct DLLEXPORT pfVec3d
{
    PFSTRUCT_DECLARE

public:
    double vec[3];

public:
    // constructors and destructors
    //CAPI:private
    pfVec3d(double _x, double _y, double _z) { set(_x, _y, _z); }
    pfVec3d() {};

public:
    // sets and gets
    //CAPI:arrayclass
    //CAPI:verb SetVec3d
    void set(double _x, double _y, double _z) {
	vec[0] = _x;
	vec[1] = _y;
	vec[2] = _z; 
    }

public:
    // other functions
    //CAPI:verb
    void copy(const pfVec3d&  _v) { *this = _v; }
    int equal(const pfVec3d&  _v) const { 
	return (vec[0] == _v[0] && 
		vec[1] == _v[1] &&
		vec[2] == _v[2]);
    }
    int almostEqual(const pfVec3d& _v, double _tol) const;

    void negate(const pfVec3d& _v) { 
	vec[0] = -_v[0];
	vec[1] = -_v[1]; 
	vec[2] = -_v[2]; 
    }

    double dot(const pfVec3d&  _v) const {
	return (vec[0] * _v[0] + 
		vec[1] * _v[1] +
		vec[2] * _v[2]);
    }

    void add(const pfVec3d& _v1, const pfVec3d& _v2) { 
	vec[0] = _v1[0] + _v2[0]; 
	vec[1] = _v1[1] + _v2[1]; 
	vec[2] = _v1[2] + _v2[2]; 
    }

    void sub(const pfVec3d& _v1, const pfVec3d& _v2) { 
	vec[0] = _v1[0] - _v2[0]; 
	vec[1] = _v1[1] - _v2[1]; 
	vec[2] = _v1[2] - _v2[2]; 
    }

    void scale(double _s, const pfVec3d& _v) { 
	vec[0] = _s * _v[0]; 
	vec[1] = _s * _v[1]; 
	vec[2] = _s * _v[2]; 
    }

    void addScaled(const pfVec3d& _v1, double _s, const pfVec3d& _v2) { 
	vec[0] = _v1[0] + _s * _v2[0]; 
	vec[1] = _v1[1] + _s * _v2[1]; 
	vec[2] = _v1[2] + _s * _v2[2]; 
    }

    void combine(double _a, const pfVec3d& _v1, double _b, const pfVec3d& _v2) { 
	vec[0] = _a * _v1[0] + _b * _v2[0]; 
	vec[1] = _a * _v1[1] + _b * _v2[1]; 
	vec[2] = _a * _v1[2] + _b * _v2[2]; 
    }

    //CAPI:verb SqrDistancePt3d
    double sqrDistance(const pfVec3d& _v) const { 
	return (PF_SQUARE(vec[0] - _v[0]) +
		PF_SQUARE(vec[1] - _v[1]) +
		PF_SQUARE(vec[2] - _v[2]));
    }

    double normalize();
    double length() const;
    //CAPI:verb DistancePt3d
    double distance(const pfVec3d& _v) const;
    void  cross(const pfVec3d&  _v1, const pfVec3d&  _v2);

    //CAPI:verb XformVec3d
    void xformVec(const pfVec3d& _v, const pfMatrix4d& _m);

    //CAPI:verb XformPt3d
    void xformPt(const pfVec3d& _v, const pfMatrix4d& _m);

    //CAPI:verb FullXformPt3d
    void fullXformPt(const pfVec3d& _v, const pfMatrix4d& _m);

    //CAPI:verb FullXformPt3dw
    double fullXformPtw(const pfVec3d& _v, const pfMatrix4d& _m);

public:
    // Operators
    double&  operator [](int i) { return vec[i]; }

    const double&  operator [](int i) const { return vec[i]; }

    int operator ==(const pfVec3d& _v) const {
        return vec[0] == _v[0] && vec[1] == _v[1] && vec[2] == _v[2];
    }
    int operator !=(const pfVec3d& _v) const {
        return !(*this == _v);
    }

public:
    // pfVec3d operators (N.B. return by value can be slow)

    pfVec3d operator -() const {
        return pfVec3d(-vec[0], -vec[1], -vec[2]);
    }

    pfVec3d operator +(const pfVec3d& _v) const {
        return pfVec3d(vec[0]+_v[0], vec[1]+_v[1], vec[2]+_v[2]);
    }

    pfVec3d operator -(const pfVec3d& _v) const {
        return pfVec3d(vec[0]-_v[0], vec[1]-_v[1], vec[2]-_v[2]);
    }

    friend inline pfVec3d operator *(double _s, const pfVec3d&);
    friend inline pfVec3d operator *(const pfVec3d& _v, double _s);
    friend inline pfVec3d operator /(const pfVec3d& _v, double _s);
    friend inline pfVec3d operator *(const pfVec3d& _v, const pfMatrix4d& _m);
    
public:
    // Assignment Operators
    pfVec3d&  operator =(const pfVec3d& _v) {
        vec[0] = _v[0]; 
	vec[1] = _v[1];
	vec[2] = _v[2]; 
	return *this;
    }

    pfVec3d& operator *=(double _s) {
        vec[0] *= _s; 
	vec[1] *= _s; 
	vec[2] *= _s; 
	return *this;
    }
    
    pfVec3d& operator /=(double _s) {
	_s = 1.0/_s; 
	return *this *= _s;
    }
    
    pfVec3d& operator +=(const pfVec3d& _v) {
        vec[0] += _v[0]; 
	vec[1] += _v[1]; 
	vec[2] += _v[2];
	return *this;
    }

    pfVec3d& operator -=(const pfVec3d& _v) {
        vec[0] -= _v[0]; 
	vec[1] -= _v[1]; 
	vec[2] -= _v[2];
	return *this;
    }
};


inline pfVec3d operator *(double _s, const pfVec3d& _v) {
    return pfVec3d(_v[0]*_s, _v[1]*_s, _v[2]*_s);
}

inline pfVec3d operator *(const pfVec3d& _v, double _s) {
    return pfVec3d(_v[0]*_s, _v[1]*_s, _v[2]*_s);
}

inline pfVec3d operator /(const pfVec3d& _v, double _s) {
    _s = 1.0f/_s;
    return pfVec3d(_v[0]*_s, _v[1]*_s, _v[2]*_s);
}

inline pfVec3d operator *(const pfVec3d& _v, const pfMatrix4d&  _m) {
    // transform as point (w=1), assuming affine transformation
    // i.e. does not use slower dst.xformFullPt().
    pfVec3d dst; dst.xformPt(_v, _m); return dst;
}

//CAPI:struct
struct DLLEXPORT pfVec4d
{
    PFSTRUCT_DECLARE

public:
    double vec[4];

public:
    // constructors and destructors
    //CAPI:arrayclass
    //CAPI:private
    pfVec4d(double _x, double _y, double _z, double _w) { set(_x, _y, _z, _w); }
    pfVec4d() {};

public:
    //CAPI:verb SetVec4d
    void set(double _x, double _y, double _z, double _w) {
	vec[0] = _x;
	vec[1] = _y;
	vec[2] = _z; 
	vec[3] = _w; 
    }

public:
    // Other functions
    //CAPI:verb
    void copy(const pfVec4d&  _v) { *this = _v; }
    int equal(const pfVec4d&  _v) const { 
	return (vec[0] == _v[0] && 
		vec[1] == _v[1] &&
		vec[2] == _v[2] &&
		vec[3] == _v[3]);
    }
    int almostEqual(const pfVec4d& _v, double _tol) const;

    void negate(const pfVec4d& _v) { 
	vec[0] = -_v[0];
	vec[1] = -_v[1]; 
	vec[2] = -_v[2]; 
	vec[3] = -_v[3]; 
    }

    double dot(const pfVec4d&  _v) const {
	return (vec[0] * _v[0] + 
		vec[1] * _v[1] +
		vec[2] * _v[2] +
		vec[3] * _v[3]);
    }

    void add(const pfVec4d& _v1, const pfVec4d& _v2) { 
	vec[0] = _v1[0] + _v2[0]; 
	vec[1] = _v1[1] + _v2[1]; 
	vec[2] = _v1[2] + _v2[2]; 
	vec[3] = _v1[3] + _v2[3]; 
    }

    void sub(const pfVec4d& _v1, const pfVec4d& _v2) { 
	vec[0] = _v1[0] - _v2[0]; 
	vec[1] = _v1[1] - _v2[1]; 
	vec[2] = _v1[2] - _v2[2]; 
	vec[3] = _v1[3] - _v2[3]; 
    }

    void scale(double _s, const pfVec4d& _v) { 
	vec[0] = _s * _v[0]; 
	vec[1] = _s * _v[1]; 
	vec[2] = _s * _v[2]; 
	vec[3] = _s * _v[3]; 
    }

    void addScaled(const pfVec4d& _v1, double _s, const pfVec4d& _v2) { 
	vec[0] = _v1[0] + _s * _v2[0]; 
	vec[1] = _v1[1] + _s * _v2[1]; 
	vec[2] = _v1[2] + _s * _v2[2]; 
	vec[3] = _v1[3] + _s * _v2[3]; 
    }

    void combine(double _a, const pfVec4d& _v1, double _b, const pfVec4d& _v2) { 
	vec[0] = _a * _v1[0] + _b * _v2[0]; 
	vec[1] = _a * _v1[1] + _b * _v2[1]; 
	vec[2] = _a * _v1[2] + _b * _v2[2]; 
	vec[3] = _a * _v1[3] + _b * _v2[3]; 
    }

    //CAPI:verb SqrDistancePt4d
    double sqrDistance(const pfVec4d& _v) const { 
	return (PF_SQUARE(vec[0] - _v[0]) +
		PF_SQUARE(vec[1] - _v[1]) +
		PF_SQUARE(vec[2] - _v[2]) +
		PF_SQUARE(vec[3] - _v[3]));
    }

    double normalize();
    double length() const;
    //CAPI:verb DistancePt4d
    double distance(const pfVec4d& _v) const;

    void xform(const pfVec4d& v, const pfMatrix4d& m);

public:
    // Operators
    double&  operator [](int i) { return vec[i]; }

    const double&  operator [](int i) const { return vec[i]; }
    
    int operator ==(const pfVec4d& _v) const {
        return (vec[0] == _v[0] && vec[1] == _v[1] &&
		vec[2] == _v[2] && vec[3] == _v[3]);
    }
    int operator !=(const pfVec4d& _v) const {
        return !(*this == _v);
    }
    
public:
    // pfVec4d operators (N.B. return by value can be slow)

    pfVec4d operator -() const {
        return pfVec4d(-vec[0], -vec[1], -vec[2], -vec[3]);
    }

    pfVec4d operator +(const pfVec4d& _v) const {
        return pfVec4d(vec[0]+_v[0], vec[1]+_v[1],
		      vec[2]+_v[2], vec[3]+_v[3]);
    }
    
    pfVec4d operator -(const pfVec4d& _v) const {
        return pfVec4d(vec[0]-_v[0], vec[1]-_v[1],
		      vec[2]-_v[2], vec[3]-_v[3]);
    }

    friend inline pfVec4d operator *(double _s, const pfVec4d&);
    friend inline pfVec4d operator *(const pfVec4d& _v, double _s);
    friend inline pfVec4d operator /(const pfVec4d& _v, double _s);
    friend inline pfVec4d operator *(const pfVec4d& _v, const pfMatrix4d& _m);
    
public:
    // Assignment Operators
    pfVec4d&  operator =(const pfVec4d& _v) {
        vec[0] = _v[0]; vec[1] = _v[1]; 
	vec[2] = _v[2]; vec[3] = _v[3]; 
	return *this;
    }

    pfVec4d& operator *=(double _s) {
        vec[0] *= _s; vec[1] *= _s; 
	vec[2] *= _s; vec[3] *= _s; 
	return *this;
    }
    
    pfVec4d& operator /=(double _s) {
	_s = 1.0f/_s; return *this *= _s;
    }
    
    pfVec4d& operator +=(const pfVec4d& _v) {
        vec[0] += _v[0]; vec[1] += _v[1]; 
	vec[2] += _v[2]; vec[3] += _v[3];
	return *this;
    }

    pfVec4d& operator -=(const pfVec4d& _v) {
        vec[0] -= _v[0]; vec[1] -= _v[1]; 
	vec[2] -= _v[2]; vec[3] -= _v[3];
	return *this;
    }
};


inline pfVec4d operator *(const pfVec4d& _v, const pfMatrix4d&  _m) {
    pfVec4d dst; dst.xform(_v, _m); return dst;
}

inline pfVec4d operator *(double _s, const pfVec4d& _v) {
    return pfVec4d(_v[0]*_s, _v[1]*_s, _v[2]*_s, _v[3]*_s);
}

inline pfVec4d operator *(const pfVec4d& _v, double _s) {
    return pfVec4d(_v[0]*_s, _v[1]*_s, _v[2]*_s, _v[3]*_s);
}

inline pfVec4d operator /(const pfVec4d& _v, double _s) {
    _s = 1.0f/_s;
    return pfVec4d(_v[0]*_s, _v[1]*_s, _v[2]*_s, _v[3]*_s);
}

//CAPI:struct
struct DLLEXPORT pfMatrix4d
{
    PFSTRUCT_DECLARE

public:
    double mat[4][4];

public:
    // constructors and destructors
    //CAPI:basename Mat4d
    //CAPI:arrayclass
    //CAPI:private
    pfMatrix4d() {};
    pfMatrix4d(double a00, double a01, double a02, double a03,
               double a10, double a11, double a12, double a13,
               double a20, double a21, double a22, double a23,
               double a30, double a31, double a32, double a33);
#if 0
#ifdef __linux__
    /* don't quite understand why this isn't defined in the IRIX version */
    pfMatrix4d(double m[4][4]) {memcpy (mat, m, sizeof(double) * 16);};
#endif
#endif
    int print(uint _travMode, uint _verbose, char *_prefix, FILE *_file);

public:
    // sets and gets
    //CAPI:verb SetMat4d
    void 	set(double *m) {
	memcpy(mat, m, sizeof(double) * 16);
    }
    //CAPI:verb GetMat4dType
    int		getMatType() const;
    //CAPI:verb SetMat4dRowVec3d
    void	setRow(int _r, const pfVec3d&  _v);

    //CAPI:verb SetMat4dRow
    void	setRow(int _r, double _x, double _y, double _z, double _w) {
	mat[_r][0] = _x; mat[_r][1] = _y; mat[_r][2] = _z; mat[_r][3] = _w;
    }
    //CAPI:verb GetMat4dRowVec3d
    void	getRow(int _r, pfVec3d&  _dst) const;
    //CAPI:verb GetMat4dRow
    void	getRow(int _r, double *_x, double *_y, double *_z, double *_w) const {
	*_x = mat[_r][0]; *_y = mat[_r][1]; *_z = mat[_r][2]; *_w = mat[_r][3];
    }
    //CAPI:verb SetMat4dColVec3d
    void	setCol(int _c, const pfVec3d&  _v);
    //CAPI:verb SetMat4dCol
    void	setCol(int _c, double _x, double _y, double _z, double _w) {
	mat[0][_c] = _x; mat[1][_c] = _y; mat[2][_c] = _z; mat[3][_c] = _w;
    }
    //CAPI:verb GetMat4dColVec3d
    void	getCol(int _c, pfVec3d& _dst) const;
    //CAPI:verb GetMat4dCol
    void	getCol(int _c, double *_x, double *_y, double *_z, double *_w) const {
	*_x = mat[0][_c]; *_y = mat[1][_c]; *_z = mat[2][_c]; *_w = mat[3][_c];
    }
    //CAPI:verb GetOrthoMat4dCoordd
    void	getOrthoCoordd(pfCoordd* _dst) const;

public:
    // Other functions
    //CAPI:verb
    void	makeIdent();
    void	makeEuler(double _hdeg, double _pdeg, double _rdeg);
    void	makeRot(double _degrees, double _x, double _y, double _z);
    void	makeTrans(double _x, double _y, double _z);
    void	makeScale(double _x, double _y, double _z);
    void	makeVecRotVec(const pfVec3d&  _v1, const pfVec3d&  _v2);
    void	makeCoordd(const pfCoordd* _c);

    // convert quaternion to and from rotation matrix representation 
    //CAPI:verb GetOrthoMat4dQuatd
    void	getOrthoQuat(pfQuatd& dst) const;
    void	makeQuat(const pfQuatd& q);

    void	copy(const pfMatrix4d&  _v) { *this = _v; }
    int		equal(const pfMatrix4d&  _m) const  {
        return (((mat)[0][0] == (_m)[0][0]) &&
		((mat)[0][1] == (_m)[0][1]) &&
		((mat)[0][2] == (_m)[0][2]) &&
		((mat)[0][3] == (_m)[0][3]) &&
		((mat)[1][0] == (_m)[1][0]) &&
		((mat)[1][1] == (_m)[1][1]) &&
		((mat)[1][2] == (_m)[1][2]) &&
		((mat)[1][3] == (_m)[1][3]) &&
		((mat)[2][0] == (_m)[2][0]) &&
		((mat)[2][1] == (_m)[2][1]) &&
		((mat)[2][2] == (_m)[2][2]) &&
		((mat)[2][3] == (_m)[2][3]) &&
		((mat)[3][0] == (_m)[3][0]) &&
		((mat)[3][1] == (_m)[3][1]) &&
		((mat)[3][2] == (_m)[3][2]) &&
		((mat)[3][3] == (_m)[3][3]));
    }

    int almostEqual(const pfMatrix4d&  _m2, double _tol) const;

public:
    // Mathematical operations on full 4X4
    //CAPI:verb
    void transpose(pfMatrix4d&  _m);
    void mult(const pfMatrix4d& _m1, const pfMatrix4d &m2);
    void add(const pfMatrix4d& _m1, const pfMatrix4d &m2);
    void sub(const pfMatrix4d& _m1, const pfMatrix4d &m2);

    // scale() multiplies full 4X4 by scalar, not a 3D scale xform
    //CAPI:header /* pfScaleMat() multiplies full 4X4 by scalar, not a 3D scale xform */
    void scale(double _s, const pfMatrix4d &_m);

    void postMult(const pfMatrix4d&  _m);
    void preMult(const pfMatrix4d&  _m);

    int invertFull(pfMatrix4d&  _m);
    void invertAff(const pfMatrix4d&  _m);
    void invertOrtho(const pfMatrix4d&  _m);
    void invertOrthoN(pfMatrix4d&  _m);
    void invertIdent(const pfMatrix4d&  _m);

public:
    //CAPI:verb
    // Transformation functions
    void preTrans(double _x, double _y, double _z, pfMatrix4d&  _m);
    void postTrans(const pfMatrix4d&  _m, double _x, double _y, double _z);

    void preRot(double _degrees, double _x, double _y, double _z, pfMatrix4d&  _m);
    void postRot(const pfMatrix4d&  _m, double _degrees, double _x, double _y, double _z);
    void preScale(double _xs, double _ys, double _zs, pfMatrix4d&  _m);
    void postScale(const pfMatrix4d&  _m, double _xs, double _ys, double _zs);

public:
    // Operators
    double*       operator [](int i)        { return &mat[i][0]; }
    const double* operator [](int i) const  { return &mat[i][0]; }
    
    int operator == (const pfMatrix4d&  _m) const {
	return this->equal(_m);
    }

    int operator != (const pfMatrix4d&  _m) const {
	return !this->equal(_m);
    }

public:
    // pfMatrix4d operators (N.B. return by value can be quite slow)
    pfMatrix4d operator *(const pfMatrix4d&  _m) const {
	pfMatrix4d dst; dst.mult(*this, _m); return dst;
    }

    pfMatrix4d operator +(const pfMatrix4d&  _m) const {
	pfMatrix4d dst; dst.add(*this, _m); return dst;
    }
    pfMatrix4d operator -(const pfMatrix4d&  _m) const {
	pfMatrix4d dst; dst.sub(*this, _m); return dst;
    }

    friend inline pfMatrix4d operator *(double _s, const pfMatrix4d&);
    friend inline pfMatrix4d operator *(const pfMatrix4d& _v, double _s);
    friend inline pfMatrix4d operator /(const pfMatrix4d& _v, double _s);

public:
    // Assignment operators
    pfMatrix4d&  operator =(const pfMatrix4d&  _m)
    {
        ((mat)[0][0] = (_m)[0][0]);
        ((mat)[0][1] = (_m)[0][1]);
        ((mat)[0][2] = (_m)[0][2]);
        ((mat)[0][3] = (_m)[0][3]);
        ((mat)[1][0] = (_m)[1][0]);
        ((mat)[1][1] = (_m)[1][1]);
        ((mat)[1][2] = (_m)[1][2]);
        ((mat)[1][3] = (_m)[1][3]);
        ((mat)[2][0] = (_m)[2][0]);
        ((mat)[2][1] = (_m)[2][1]);
        ((mat)[2][2] = (_m)[2][2]);
        ((mat)[2][3] = (_m)[2][3]);
        ((mat)[3][0] = (_m)[3][0]);
        ((mat)[3][1] = (_m)[3][1]);
        ((mat)[3][2] = (_m)[3][2]);
        ((mat)[3][3] = (_m)[3][3]);
        return *this;
    }

    pfMatrix4d&  operator *=(const pfMatrix4d&  _m)  {
	this->postMult(_m); return *this; 
    }

    pfMatrix4d&  operator *=(double _s);
    pfMatrix4d&  operator /=(double _s);
    pfMatrix4d&  operator +=(const pfMatrix4d&  _m);
    pfMatrix4d&  operator -=(const pfMatrix4d&  _m);
};

inline pfMatrix4d operator *(double _s, const pfMatrix4d& _m) {
    pfMatrix4d dst; dst.scale(_s, _m); return dst;
}

inline pfMatrix4d operator *(const pfMatrix4d& _m, double _s) {
    pfMatrix4d dst; dst.scale(_s, _m); return dst;
}

inline pfMatrix4d operator /(const pfMatrix4d& _m, double _s) {
    pfMatrix4d dst; dst.scale(1.0f/_s, _m); return dst;
}

extern "C" {     // EXPORT to CAPI
/*
 * pfQuatd derives from pfVec4d.
 * pfVec4d routines can also operate on pfQuatds.
 */
} // extern "C"

//CAPI:struct
struct DLLEXPORT pfQuatd : public pfVec4d
{
    PFSTRUCT_DECLARE

public:
    // constructors and destructors
    //CAPI:arrayclass
    //CAPI:private
    pfQuatd(double _x, double _y, double _z, double _w) { set(_x, _y, _z, _w); }
    pfQuatd() {};

    //CAPI:private
    pfQuatd(const double v[4]) { set(v[0], v[1], v[2], v[3]); }
    //CAPI:private
    pfQuatd(const pfMatrix4d &m) { makeRot(m); }
    //CAPI:private
    pfQuatd(const pfVec3d &axis, float _angle) { makeRot(_angle, axis[0], axis[1], axis[2]); }
    //CAPI:private
    pfQuatd(const pfVec3d &rotateFrom, const pfVec3d rotateTo) { makeVecRotVec(rotateFrom, rotateTo); }

public:
    void getRot(double *_angle, double *_x, double *_y, double *_z) const;
    //CAPI:verb
    void makeRot(double _angle, double _x, double _y, double _z);

    void getRot(pfMatrix4d &m);
    //CAPI:verb MakeMatRotQuatd
    void makeRot(const pfMatrix4d &m);
    //CAPI:verb
    void makeVecRotVec(const pfVec3d &rotateFrom, const pfVec3d rotateTo);

public:
    // Other functions
    //CAPI:verb
    void conj(const pfQuatd& _v) { 
	vec[0] = -_v[0];
	vec[1] = -_v[1]; 
	vec[2] = -_v[2]; 
	vec[3] =  _v[3]; 
    }

    double length() const {
	return (vec[0]*vec[0]+vec[1]*vec[1]+vec[2]*vec[2]+vec[3]*vec[3]);	
    }
    
public:
    //CAPI:verb
    void mult(const pfQuatd& q1, const pfQuatd& q2);
    void div(const pfQuatd& q1, const pfQuatd& q2);
    void invert(const pfQuatd& q1);
    void exp(const pfQuatd& _q);
    void log(const pfQuatd& _q);

    void slerp(double _t, const pfQuatd& _q1, const pfQuatd& _q2);
    void squad(double _t, const pfQuatd& _q1, const pfQuatd& _q2, const pfQuatd& _a, const pfQuatd& _b);

    //CAPI:noverb
    void meanTangent(const pfQuatd& _q1, const pfQuatd& _q2, const pfQuatd& _q3);

public:
    // pfQuatd operators (N.B. return by value can be slow)
    pfQuatd operator *(const pfQuatd&  _m) const {
	pfQuatd dst; dst.mult(*this, _m); return dst;
    }

    pfQuatd operator /(const pfQuatd&  _m) const {
	pfQuatd dst; dst.div(*this, _m); return dst;
    }

public:
    // Assignment operators
    pfQuatd&  operator *=(const pfQuatd& _q) {
	this->mult(*this, _q); return *this;
    }
    pfQuatd&  operator /=(const pfQuatd& _q) {
	this->div(*this, _q); return *this;
    }
};

extern "C" {
extern DLLEXPORT pfMatrix pfIdentMat;
extern DLLEXPORT pfMatrix4d pfIdentMat4d;
}

struct DLLEXPORT pfCoord {
    pfVec3	xyz;
    pfVec3	hpr;
};

struct DLLEXPORT pfCoordd {
    pfVec3d	xyz;
    pfVec3d	hpr;
};


extern "C" {     // EXPORT to CAPI

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

} // END of 
 
#define PFMATSTACK ((pfMatStack*)_pfCurrentBuffer->pf_indexUpdatable(this))

#define PFMATSTACKBUFFER ((pfMatStack*)buf->pf_indexUpdatable(this))

class DLLEXPORT pfMatStack : public pfObject
{
    //CAPI:basename MStack
    //CAPI:argname  mst
public:
    // Constructors, destructors
    pfMatStack(int _size);

    //CAPI:private
    pfMatStack();
    virtual ~pfMatStack();

public:
    // per class functions;
    static void	   init();
    static pfType* getClassType() { return classType; }

public:
    // virtual functions

    virtual int compare(const pfMemory *_mem) const;
    virtual int copy(const pfMemory *_src);
    virtual int print(uint _travMode, uint _verbose, char *_prefix, FILE *_file);

public:
    // sets and gets
    void	get(pfMatrix& _m) const {
	_m = *sp;
    }

    pfMatrix*	getTop() const {
	return sp;
    }

    int		getDepth() const {
	return depth;
    }
    
    //CAPI:private
    int		getStackSize(int _size) const {
        return (int)sizeof(pfMatrix) * _size;
    }
    void	setStack(void *stack, int _size);

public:	
    // other functions
    //CAPI:verb

    void	reset();
    //CAPI:err FALSE
    int		push();
    //CAPI:err FALSE
    int		pop();

    void	load(const pfMatrix& _m) {
	*sp = _m;
    }

    void	preMult(const pfMatrix& _m) {
	sp->preMult(_m);
    }
    void	postMult(const pfMatrix& _m) {
	sp->postMult(_m);
    }

    void	preTrans(float _x, float _y, float _z) {
	sp->preTrans(_x, _y, _z, *sp);
    }

    void	postTrans(float _x, float _y, float _z) {
	sp->postTrans(*sp, _x, _y, _z);
    }

    void	preRot(float _degrees, float _x, float _y, float _z) {
	sp->preRot(_degrees, _x, _y, _z, *sp);
    }
    
    void	postRot(float _degrees, float _x, float _y, float _z) {
	sp->postRot(*sp, _degrees, _x, _y, _z);
    }

    void	preScale(float _xs, float _ys, float _zs) {
	sp->preScale(_xs, _ys, _zs, *sp);
    }
    void	postScale(float _xs, float _ys, float _zs) {
	sp->postScale(*sp, _xs, _ys, _zs);
    }

private:
    pfMatrix    *base;	// Matrix stack 
    pfMatrix    *__UNUSED_end;	// End of stack 
    pfMatrix    *sp;	// Stack pointer 
    int	   	depth;	// redundant info for quick tests 
    int	   	stackSize; // size of stack 

private:
    static pfType *classType;
};

extern "C" {     // EXPORT to CAPI

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

} // extern "C" END of C include export

#endif //  __PF_LINMATH_H__

