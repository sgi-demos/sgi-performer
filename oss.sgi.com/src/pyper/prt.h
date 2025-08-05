//
// $Source: /oss/CVS/cvs/performer/src/pyper/prt.h,v $
// $Revision: 1.1 $
// $Author: flynnt $
// $Date: 2001/05/21 21:40:00 $
//


#ifndef PRT_H
#define PRT_H

#include <Performer/pf.h>


#define PRTMAT(M) \
  printf \
  ( \
    "matrix %s:\n" \
    "[%8.3f %8.3f %8.3f %8.3f]\n" \
    "[%8.3f %8.3f %8.3f %8.3f]\n" \
    "[%8.3f %8.3f %8.3f %8.3f]\n" \
    "[%8.3f %8.3f %8.3f %8.3f]\n", \
    # M, \
    (M)[0][0], (M)[0][1], (M)[0][2], (M)[0][3], \
    (M)[1][0], (M)[1][1], (M)[1][2], (M)[1][3], \
    (M)[2][0], (M)[2][1], (M)[2][2], (M)[2][3], \
    (M)[3][0], (M)[3][1], (M)[3][2], (M)[3][3] \
  ); 

#define PRTVEC3(V) \
  printf \
  ( \
    "vector %s:\n" \
    "[%8.3f %8.3f %8.3f]\n", \
    # V, \
    (V)[0], (V)[1], (V)[2] \
  );

#define PRTVEC2(V) \
  printf \
  ( \
    "vector %s:\n" \
    "[%8.3f %8.3f]\n", \
    # V, \
    (V)[0], (V)[1] \
  );


#define PRTQUAT(Q) \
  { \
    float angle,x,y,z; \
    (Q).getRot(&angle, &x, &y, &z); \
    printf \
    ( \
      "quaternion %s: %8.2f %8.2f %8.2f %8.2f\n", \
      # Q, \
      (Q)[0], (Q)[1], (Q)[2], (Q)[3] \
    ); \
    printf \
    ( \
      "angle %f, axis %f,%f,%f\n", \
      angle, x, y, z \
    ); \
  }


#endif

// $Log: prt.h,v $
// Revision 1.1  2001/05/21 21:40:00  flynnt
// Doing some cleanup and adding the pfgtk example and the python wrapper for
// Performer (pyper).
//
// Revision 1.2  2001/03/19 12:32:31  bram
// Fixed spline interpolation of pfVec3
// Stamped out usage of ASSERT. Should use assert instead.
//
