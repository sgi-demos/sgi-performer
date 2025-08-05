//
// $Source: /oss/CVS/cvs/performer/src/pyper/v3int.hh,v $
// $Revision: 1.1 $
// $Author: flynnt $
// $Date: 2001/05/21 21:40:00 $
// Purpose:
//
// A Vector3Interpolator class, the collection of events which make up a
// 3-valued object property animation (like position/size)
//
// (c) 1998 by Polar Pyramid.
//

#if !defined(VECTOR3INTERPOLATOR_HH)
#define VECTOR3INTERPOLATOR_HH

#include "aniint.hh"

//
// pfVec3Interpolator
//

class pfVec3Interpolator;
typedef pfVec3Interpolator *pfVec3InterpolatorPtr;

class pfVec3Interpolator : public AniInterpolator
{
public:
  pfVec3Interpolator();
  virtual ~pfVec3Interpolator();

  virtual void AddEvent(float time, const pfVec3 &val);
  virtual void AddEvent(AniEventPtr event);                  

  virtual pfVec3 Evaluate(float time);
  virtual pfVec3 EvaluateDerivative(float time);
  void    Evaluation(float time, pfVec3 &v);
  void SetDerivatives(void);
  bool Store(const std::string &fname) const;
  virtual void Store(FILE *f) const;
  bool Load(const std::string &fname);
  virtual void Load(FILE *f);
private:
  void AdjustDerivativesFromTime(void);
  pfVec3 EvaluatepfVec3(float time);
  pfVec3 EvaluateTangent(float time);
};

#endif

// $Log: v3int.hh,v $
// Revision 1.1  2001/05/21 21:40:00  flynnt
// Doing some cleanup and adding the pfgtk example and the python wrapper for
// Performer (pyper).
//
// Revision 1.5  2001/04/10 13:50:42  bram
// Avoided return by value of ::Evaluate()
// We now have an alternative method ::Evaluation() that passes
// the result as a function parameter instead.
// This avoids problems with swig.
//
// Revision 1.4  2001/02/27 16:09:43  bram
// Added serailizing for string interpolators
//
// Revision 1.3  2001/02/27 10:10:31  bram
// added importer for saranav path files
//
// Revision 1.2  2001/02/22 15:47:37  bram
// Improved transform animator.
// Added serializing for ani interpolators.
//
// Revision 1.1.1.1  2000/08/23 08:02:42  bram
// Added a PyPer version to the repository
//
// Revision 1.1.1.1  2000/08/22 17:04:20  bram
// Added pyper
//
//
