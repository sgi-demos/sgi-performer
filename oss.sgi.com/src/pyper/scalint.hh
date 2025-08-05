//
// $Source: /oss/CVS/cvs/performer/src/pyper/scalint.hh,v $
// $Revision: 1.1 $
// $Author: flynnt $
// $Date: 2001/05/21 21:40:00 $
// Purpose:
//
// A ScalarInterpolator class, the collection of events which make up a
// 3-valued object property animation (like position/size)
//
// (c) 1998 by Polar Pyramid.
//

#if !defined(SCALARINTERPOLATOR_HH)
#define SCALARINTERPOLATOR_HH

#include "aniint.hh"

//
// ScalarInterpolator
//

class ScalarInterpolator;
typedef ScalarInterpolator *ScalarInterpolatorPtr;

class ScalarInterpolator : public AniInterpolator
{
public:
  ScalarInterpolator();
  virtual ~ScalarInterpolator();
  virtual void AddEvent(AniEventPtr event);                
  virtual void AddEvent(float time, const float value);
  virtual float Evaluate(float time);
  bool Store(const std::string &fname) const;
  virtual void Store(FILE *f) const;
  bool Load(const std::string &fname);
  virtual void Load(FILE *f);
private:
  float EvaluateScalar(float time);
};

#endif

// $Log: scalint.hh,v $
// Revision 1.1  2001/05/21 21:40:00  flynnt
// Doing some cleanup and adding the pfgtk example and the python wrapper for
// Performer (pyper).
//
// Revision 1.5  2001/02/27 16:09:43  bram
// Added serailizing for string interpolators
//
// Revision 1.4  2001/02/27 10:10:30  bram
// added importer for saranav path files
//
// Revision 1.3  2001/02/22 15:47:36  bram
// Improved transform animator.
// Added serializing for ani interpolators.
//
// Revision 1.2  2001/01/22 08:07:32  bram
// misc fixes
//
// Revision 1.1.1.1  2000/08/23 08:02:41  bram
// Added a PyPer version to the repository
//
// Revision 1.1.1.1  2000/08/22 17:04:20  bram
// Added pyper
//

