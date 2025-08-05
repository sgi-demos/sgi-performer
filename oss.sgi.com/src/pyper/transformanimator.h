//
// $Source: /oss/CVS/cvs/performer/src/pyper/transformanimator.h,v $
// $Revision: 1.1 $
// $Author: flynnt $
// $Date: 2001/05/21 21:40:00 $
//

#ifndef TRANSFORMANIMATOR_H
#define TRANSFORMANIMATOR_H


class pfDCS;

#include "v3int.hh"
#include "quatint.hh"
#include "scalint.hh"


#define TRAJECTORY_SIZE    1024

class pfGeode;
class pfGeoSet;

class TransformAnimator
{
  public:
    TransformAnimator(pfDCS &dcs);
    ~TransformAnimator();
    void UseSplines(bool v);
    void AddEvent(float time);
    void Evaluate(float time);
    float GetClosestKeyTime(float t) const;
    pfNode *GetTrajectoryGeometry(void) const;
    void ShowTrajectoryAsInverted(bool inv) { TrajectoryIsInverted=inv; }
    void SetDerivativeScale(float scale);
    void SetCircular(bool circ);
    void Clear(void);
    bool Store(const std::string &fname) const;
    void Store(FILE *f) const;
    bool Load(const std::string &fname);
    void Load(FILE *f);
    bool Import(const std::string &fname, float assumed_fps=60.0);

  protected:
    pfDCS &TargetDCS;
    pfVec3Interpolator   PosInterp;
    pfQuatInterpolator   OriInterp;
    ScalarInterpolator   SclInterp;  // Only uniform scaling!
    pfGeode             *TrajectoryGeode;
    pfGeoSet            *TrajectoryGeoSet;
    pfVec3              *TrajectoryVertices;
    pfVec4              *TrajectoryColors;
    bool		 TrajectoryIsInverted;

    void CreateTrajectory(void);
    void UpdateTrajectory(void);
};

#endif

// $Log: transformanimator.h,v $
// Revision 1.1  2001/05/21 21:40:00  flynnt
// Doing some cleanup and adding the pfgtk example and the python wrapper for
// Performer (pyper).
//
// Revision 1.8  2001/03/28 08:57:06  jorrit
// Update taken from Bram. Some methods added
//
// Revision 1.7  2001/03/19 12:32:31  bram
// Fixed spline interpolation of pfVec3
// Stamped out usage of ASSERT. Should use assert instead.
//
// Revision 1.6  2001/02/27 10:10:30  bram
// added importer for saranav path files
//
// Revision 1.5  2001/02/26 07:54:47  bram
// fixed path recording
//
// Revision 1.4  2001/02/22 15:47:37  bram
// Improved transform animator.
// Added serializing for ani interpolators.
//
// Revision 1.3  2001/02/07 14:07:03  bram
// added trajectory
//
// Revision 1.2  2000/11/21 13:05:21  bram
// improved transform animator
//
// Revision 1.1  2000/11/16 13:51:56  bram
// added transform animator
//
