/*
 * $Source: /oss/CVS/cvs/performer/src/pyper/hermite.hh,v $
 * $Revision: 1.1 $
 * $Author: flynnt $
 * $Date: 2001/05/21 21:40:00 $
 * Purpose:
 *
 * (c) 1996, 1997, 1998 by Polar Pyramid.
 */


#if !defined(HERMITE_HH)
#define HERMITE_HH

#define mulher(MAT1, MAT2) \
  MAT2[0][0] = 2.0*MAT1[0][0] - 2.0*MAT1[1][0] + MAT1[2][0] + MAT1[3][0]; \
  MAT2[1][0] = -3.0*MAT1[0][0] + 3.0*MAT1[1][0] - 2.0*MAT1[2][0] - MAT1[3][0]; \
  MAT2[2][0] = MAT1[2][0] ; \
  MAT2[3][0] = MAT1[0][0] ; \
  MAT2[0][1] = 2.0*MAT1[0][1] - 2.0*MAT1[1][1] + MAT1[2][1] + MAT1[3][1]; \
  MAT2[1][1] = -3.0*MAT1[0][1] + 3.0*MAT1[1][1] - 2.0*MAT1[2][1] - MAT1[3][1]; \
  MAT2[2][1] = MAT1[2][1] ; \
  MAT2[3][1] = MAT1[0][1] ; \
  MAT2[0][2] = 2.0*MAT1[0][2] - 2.0*MAT1[1][2] + MAT1[2][2] + MAT1[3][2]; \
  MAT2[1][2] = -3.0*MAT1[0][2] + 3.0*MAT1[1][2] - 2.0*MAT1[2][2] - MAT1[3][2]; \
  MAT2[2][2] = MAT1[2][2] ; \
  MAT2[3][2] = MAT1[0][2] ;


inline pfVec3 cubpos(float m[4][3], float t)
{
  float f1 = t*t, f2 = t*f1;
  return pfVec3
  (
    f2*m[0][0]+f1*m[1][0]+t*m[2][0]+m[3][0],
    f2*m[0][1]+f1*m[1][1]+t*m[2][1]+m[3][1],
    f2*m[0][2]+f1*m[1][2]+t*m[2][2]+m[3][2]
  );
}


inline pfVec3 cubtan(float m[4][3], float t)
{
  float f1 = 3.0f*t*t, f2 = 2.0f*t;
  return pfVec3
  ( 
    f1*m[0][0]+f2*m[1][0]+m[2][0],
    f1*m[0][1]+f2*m[1][1]+m[2][1],
    f1*m[0][2]+f2*m[1][2]+m[2][2]
  );
}

#endif

