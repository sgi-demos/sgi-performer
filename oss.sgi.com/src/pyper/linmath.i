
// This file contains the public interface from the Performer
// header file pr/pfLinMath.h
// This Performer header file is (c) Silicon Graphics, Inc.
// Binding specific code is (c) SARA.

%{
#include <Performer/pr/pfLinMath.h>
%}

%subsection "linmath"

struct pfMatrix;
struct pfQuat;
class pfHit;

%subsubsection "Vector"

struct pfVec2
{
public:
    %addmethods
    {
      char *__str__() 
      {
        static char temp[256];
        sprintf(temp, "[%g, %g]", self->vec[0], self->vec[1]);
        return temp;
      }

      char *__repr__()
      {
        static char temp[256];
        sprintf(temp, "[%g, %g]", self->vec[0], self->vec[1]);
        return temp;
      }
 
      float __getitem__(int i) { return self->vec[i]; }
      void __setitem__(int i, float v) { self->vec[i] = v; }

      /* python specific operators */

      %new pfVec2 __add__(pfVec2 other)
      {
        return (*self + other);  
      }
      %new pfVec2 __sub__(pfVec2 other)
      {
        return (*self - other);
      }
      %new pfVec2 __neg__() 
      { 
        return (- *self);
      }
      int __cmp__(pfVec2 other)
      {
        if (self->equal(other)) return 0; 
        return (self->length() < other.length())? -1 : 1;
      }

    }

    pfVec2();
    ~pfVec2();

    void set(float _x, float _y);

    void copy(const pfVec2&  _v);

    int equal(const pfVec2&  _v) const;

    int almostEqual(const pfVec2& _v, float _tol) const;

    void negate(const pfVec2& _v);

    float dot(const pfVec2&  _v) const;

    void add(const pfVec2& _v1, const pfVec2& _v2);

    void sub(const pfVec2& _v1, const pfVec2& _v2);

    void scale(float _s, const pfVec2& _v);

    void addScaled(const pfVec2& _v1, float _s, const pfVec2& _v2);

    void combine(float _a, const pfVec2& _v1, float _b, const pfVec2& _v2);

    float sqrDistance(const pfVec2& _v) const;

    float normalize();

    float length() const;

    float distance(const pfVec2& _v) const;

/*
    // Operators
    float&  operator [](int i);
    const float&  operator [](int i) const;

    int operator ==(const pfVec2& _v) const;
    int operator !=(const pfVec2& _v) const;

    pfVec2 operator -() const;

    pfVec2 operator +(const pfVec2& _v) const;
    
    pfVec2 operator -(const pfVec2& _v) const;

    friend inline pfVec2 operator *(float _s, const pfVec2&);
    friend inline pfVec2 operator *(const pfVec2& _v, float _s);
    friend inline pfVec2 operator /(const pfVec2& _v, float _s);

    // Assignment Operators
    pfVec2&  operator =(const pfVec2& _v);

    pfVec2& operator *=(float _s);
    
    pfVec2& operator /=(float _s);
    
    pfVec2& operator +=(const pfVec2& _v);

    pfVec2& operator -=(const pfVec2& _v);
*/

};

struct pfVec3
{

public:

    %addmethods
    {
      char *__str__() 
      {
        static char temp[256];
        sprintf(temp, "[%g, %g, %g]", self->vec[0], self->vec[1], self->vec[2]);
        return temp;
      }
      char *__repr__() 
      {
        static char temp[256];
        sprintf(temp, "[%g, %g, %g]", self->vec[0], self->vec[1], self->vec[2]);
        return temp;
      }

      float __getitem__(int i) { return self->vec[i]; }
      void __setitem__(int i, float v) { self->vec[i] = v; }

      /* python specific operators */

      %new pfVec3 __add__(pfVec3 other)
      {
        return (*self + other);  
      }
      %new pfVec3 __sub__(pfVec3 other)
      {
        return (*self - other);
      }
      %new pfVec3 __neg__() 
      { 
        return (- *self);
      }
      int __cmp__(pfVec3 other)
      {
        if (self->equal(other)) return 0; 
        return (self->length() < other.length())? -1 : 1;
      }



    }


#if 0
    %name(pfVec3_void) void pfVec3();

    %name(pfVec3_fff)  void pfVec3(float x, float y, float z);
#else
    pfVec3();
#endif


    ~pfVec3();

    void set(float _x, float _y, float _z);

    // other functions
    void copy(const pfVec3&  _v);
    int equal(const pfVec3&  _v) const;
    int almostEqual(const pfVec3& _v, float _tol) const;
    void negate(const pfVec3& _v);
    float dot(const pfVec3&  _v) const;
    void add(const pfVec3& _v1, const pfVec3& _v2);
    void sub(const pfVec3& _v1, const pfVec3& _v2);
    void scale(float _s, const pfVec3& _v);
    void addScaled(const pfVec3& _v1, float _s, const pfVec3& _v2);
    void combine(float _a, const pfVec3& _v1, float _b, const pfVec3& _v2);
    float sqrDistance(const pfVec3& _v) const;
    float normalize();
    float length() const;
    float distance(const pfVec3& _v) const;
    void  cross(const pfVec3&  _v1, const pfVec3&  _v2);
    void xformVec(const pfVec3& _v, const pfMatrix& _m);
    void xformPt(const pfVec3& _v, const pfMatrix& _m);
    void fullXformPt(const pfVec3& _v, const pfMatrix& _m);

/*
    // Operators
    float&  operator [](int i);

    const float&  operator [](int i) const;

    int operator ==(const pfVec3& _v) const;
    int operator !=(const pfVec3& _v) const;
    pfVec3 operator -() const;
    pfVec3 operator +(const pfVec3& _v) const;
    pfVec3 operator -(const pfVec3& _v) const;

    friend inline pfVec3 operator *(float _s, const pfVec3&);
    friend inline pfVec3 operator *(const pfVec3& _v, float _s);
    friend inline pfVec3 operator /(const pfVec3& _v, float _s);
    friend inline pfVec3 operator *(const pfVec3& _v, const pfMatrix& _m);
    
    pfVec3&  operator =(const pfVec3& _v);
    pfVec3& operator *=(float _s);
    pfVec3& operator /=(float _s);
    pfVec3& operator +=(const pfVec3& _v);
    pfVec3& operator -=(const pfVec3& _v);
*/
};


#if 0
%pragma(python) code="
  class pfVec3(pfVec3) :
    def pfVec3(self,*args):
      if len(args)==3 :
        self.this = apply(pfVec3.pfVec3_void, args)
      else:
        self.this = apply(pfVec3.pfVec3_iii,  args)
      self.thisown = 1
"
#endif


struct pfVec4
{

public:
    %addmethods
    {
      char *__str__() 
      {
        static char temp[256];
        sprintf(temp, "[%g, %g, %g, %g]", self->vec[0], self->vec[1], self->vec[2], self->vec[3]);
        return temp;
      }
      char *__repr__() 
      {
        static char temp[256];
        sprintf(temp, "[%g, %g, %g, %g]", self->vec[0], self->vec[1], self->vec[2], self->vec[3]);
        return temp;
      }
      float __getitem__(int i) { return self->vec[i]; }
      void __setitem__(int i, float v) { self->vec[i] = v; }
    
      /* python specific operators */

      %new pfVec4 __add__(pfVec4 other)
      {
        return (*self + other); 
      }
      %new pfVec4 __sub__(pfVec4 other)
      {
        return (*self - other);
      }
      %new pfVec4 __neg__() 
      { 
        return (- *self);
      }
      int __cmp__(pfVec4 other)
      {
        if (self->equal(other)) return 0; 
        return (self->length() < other.length())? -1 : 1;
      }


    }

    // constructors and destructors
    pfVec4();
    ~pfVec4();

    void set(float _x, float _y, float _z, float _w);

    // Other functions
    void copy(const pfVec4&  _v);
    int equal(const pfVec4&  _v) const;
    int almostEqual(const pfVec4& _v, float _tol) const;
    void negate(const pfVec4& _v);
    float dot(const pfVec4&  _v) const;
    void add(const pfVec4& _v1, const pfVec4& _v2);
    void sub(const pfVec4& _v1, const pfVec4& _v2);
    void scale(float _s, const pfVec4& _v);
    void addScaled(const pfVec4& _v1, float _s, const pfVec4& _v2);
    void combine(float _a, const pfVec4& _v1, float _b, const pfVec4& _v2);
    float sqrDistance(const pfVec4& _v) const;
    float normalize();
    float length() const;
    float distance(const pfVec4& _v) const;
    void xform(const pfVec4& v, const pfMatrix& m);

/*
    // Operators
    float&  operator [](int i);
    const float&  operator [](int i) const;
    int operator ==(const pfVec4& _v) const;
    int operator !=(const pfVec4& _v) const;
    
    pfVec4 operator -() const;
    pfVec4 operator +(const pfVec4& _v) const;
    pfVec4 operator -(const pfVec4& _v) const;
    friend inline pfVec4 operator *(float _s, const pfVec4&);
    friend inline pfVec4 operator *(const pfVec4& _v, float _s);
    friend inline pfVec4 operator /(const pfVec4& _v, float _s);
    friend inline pfVec4 operator *(const pfVec4& _v, const pfMatrix& _m);
    
    // Assignment Operators
    pfVec4&  operator =(const pfVec4& _v);
    pfVec4& operator *=(float _s);
    pfVec4& operator /=(float _s);
    pfVec4& operator +=(const pfVec4& _v);
    pfVec4& operator -=(const pfVec4& _v);
*/
};

%subsubsection "Matrix"

struct pfMatrix
{

public:

  %addmethods
  {
    char *__str__() 
    {
      static char temp[512];
      sprintf
      (
        temp,
        "[%8.2f %8.2f %8.2f %8.2f]\n" 
        "[%8.2f %8.2f %8.2f %8.2f]\n"
        "[%8.2f %8.2f %8.2f %8.2f]\n"
        "[%8.2f %8.2f %8.2f %8.2f]\n",
        self->mat[0][0], self->mat[0][1], self->mat[0][2], self->mat[0][3],
        self->mat[1][0], self->mat[1][1], self->mat[1][2], self->mat[1][3],
        self->mat[2][0], self->mat[2][1], self->mat[2][2], self->mat[2][3],
        self->mat[3][0], self->mat[3][1], self->mat[3][2], self->mat[3][3]
      );
      return temp;
    }

    char *__repr__()
    {
      static char temp[512];
      sprintf
      (
        temp,
        "[%8.2f %8.2f %8.2f %8.2f]\n" 
        "[%8.2f %8.2f %8.2f %8.2f]\n"
        "[%8.2f %8.2f %8.2f %8.2f]\n"
        "[%8.2f %8.2f %8.2f %8.2f]\n",
        self->mat[0][0], self->mat[0][1], self->mat[0][2], self->mat[0][3],
        self->mat[1][0], self->mat[1][1], self->mat[1][2], self->mat[1][3],
        self->mat[2][0], self->mat[2][1], self->mat[2][2], self->mat[2][3],
        self->mat[3][0], self->mat[3][1], self->mat[3][2], self->mat[3][3]
      );
      return temp;
    }

      /* python specific operators */

      %new pfMatrix __add__(pfMatrix other)
      {
        return (*self + other);  
      }

      %new pfMatrix __sub__(pfMatrix other)
      {
        return (*self - other);
      }

      %new pfMatrix __mul__(pfMatrix other)
      {
        return (*self * other);
      }

      int __cmp__(pfMatrix other)
      {
        return (self->equal(other))? 0 : 1; 
      }


  }

    // constructors and destructors
    pfMatrix();
    ~pfMatrix();

    void 	set(float *m);
    int		getMatType() const;
    void	setRow(int _r, const pfVec3&  _v);
//    void	setRow(int _r, float _x, float _y, float _z, float _w);
    void	getRow(int _r, pfVec3&  _dst) const;
//    void	getRow(int _r, float *_x, float *_y, float *_z, float *_w) const;
    void	setCol(int _c, const pfVec3&  _v);
//    void	setCol(int _c, float _x, float _y, float _z, float _w);
    void	getCol(int _c, pfVec3& _dst) const;
//    void	getCol(int _c, float *_x, float *_y, float *_z, float *_w) const;
    void	getOrthoCoord(pfCoord* _dst) const;

    void	makeIdent();
    void	makeEuler(float _hdeg, float _pdeg, float _rdeg);
    void	makeRot(float _degrees, float _x, float _y, float _z);
    void	makeTrans(float _x, float _y, float _z);
    void	makeScale(float _x, float _y, float _z);
    void	makeVecRotVec(const pfVec3&  _v1, const pfVec3&  _v2);
    void	makeCoord(const pfCoord* _c);

    // convert quaternion to and from rotation matrix representation 
    void	getOrthoQuat(pfQuat& dst) const;
    void	makeQuat(const pfQuat& q);

    void	copy(const pfMatrix&  _v);
    int		equal(const pfMatrix&  _m) const;
    int almostEqual(const pfMatrix&  _m2, float _tol) const;

    // Mathematical operations on full 4X4
    void transpose(pfMatrix&  _m);
    void mult(const pfMatrix& _m1, const pfMatrix &m2);
    void add(const pfMatrix& _m1, const pfMatrix &m2);
    void sub(const pfMatrix& _m1, const pfMatrix &m2);

    // scale() multiplies full 4X4 by scalar, not a 3D scale xform
    void scale(float _s, const pfMatrix &_m);

    void postMult(const pfMatrix&  _m);
    void preMult(const pfMatrix&  _m);

    int invertFull(pfMatrix&  _m);
    void invertAff(const pfMatrix&  _m);
    void invertOrtho(const pfMatrix&  _m);
    void invertOrthoN(pfMatrix&  _m);
    void invertIdent(const pfMatrix&  _m);

    // Transformation functions
    void preTrans(float _x, float _y, float _z, pfMatrix&  _m);
    void postTrans(const pfMatrix&  _m, float _x, float _y, float _z);

    void preRot(float _degrees, float _x, float _y, float _z, pfMatrix&  _m);
    void postRot(const pfMatrix&  _m, float _degrees, float _x, float _y, float _z);
    void preScale(float _xs, float _ys, float _zs, pfMatrix&  _m);
    void postScale(const pfMatrix&  _m, float _xs, float _ys, float _zs);

/*
    // Operators
    float*       operator [](int i);
    const float* operator [](int i) const;
    
    int operator == (const pfMatrix&  _m) const;
    int operator != (const pfMatrix&  _m) const;
    pfMatrix operator *(const pfMatrix&  _m) const;
    pfMatrix operator +(const pfMatrix&  _m) const;
    pfMatrix operator -(const pfMatrix&  _m) const;

    friend inline pfMatrix operator *(float _s, const pfMatrix&);
    friend inline pfMatrix operator *(const pfMatrix& _v, float _s);
    friend inline pfMatrix operator /(const pfMatrix& _v, float _s);

    // Assignment operators
    pfMatrix&  operator =(const pfMatrix&  _m);
    pfMatrix&  operator *=(const pfMatrix&  _m);
    pfMatrix&  operator *=(float _s);
    pfMatrix&  operator /=(float _s);
    pfMatrix&  operator +=(const pfMatrix&  _m);
    pfMatrix&  operator -=(const pfMatrix&  _m);
*/
};

%subsubsection "Quaternion"

struct pfQuat : public pfVec4
{

public:

//    pfQuat(float _x, float _y, float _z, float _w);

    pfQuat();
    ~pfQuat();

    void getRot(float *_angle, float *_x, float *_y, float *_z) const;
    void makeRot(float _angle, float _x, float _y, float _z);

    // Other functions
    void conj(const pfQuat& _v);
    float length() const;
    void mult(const pfQuat& q1, const pfQuat& q2);
    void div(const pfQuat& q1, const pfQuat& q2);
    void invert(const pfQuat& q1);
    void exp(const pfQuat& _q);
    void log(const pfQuat& _q);

    void slerp(float _t, const pfQuat& _q1, const pfQuat& _q2);
    void squad(float _t, const pfQuat& _q1, const pfQuat& _q2, const pfQuat& _a, const pfQuat& _b);

    void meanTangent(const pfQuat& _q1, const pfQuat& _q2, const pfQuat& _q3);

    // pfQuat operators (N.B. return by value can be slow)
/*
    pfQuat operator *(const pfQuat&  _m) const;
    pfQuat operator /(const pfQuat&  _m) const;


    // Assignment operators
    pfQuat&  operator *=(const pfQuat& _q);
    pfQuat&  operator /=(const pfQuat& _q);
*/
};

%subsubsection "Coordinate"

struct pfCoord 
{
  public:
    pfCoord();
    ~pfCoord();

    pfVec3	xyz;
    pfVec3	hpr;
};






