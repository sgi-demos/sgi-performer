//
// $Source: /oss/CVS/cvs/performer/src/pyper/quatint.hh,v $
// $Revision: 1.1 $
// $Author: flynnt $
// $Date: 2001/05/21 21:40:00 $
// Purpose:
//
// A QuatInterpolator class, the collection of events which make up a
// rotation animation using quaternions
//
// (c) 1998 by Polar Pyramid.
//

#if !defined(QUATINTERPOLATOR_HH)
#define QUATINTERPOLATOR_HH

#include "aniint.hh"

//
// QuatInterpolator
//

class pfQuatInterpolator;
typedef pfQuatInterpolator *pfQuatInterpolatorPtr;

class pfQuatInterpolator : public AniInterpolator
{
public:
  pfQuatInterpolator();
  virtual ~pfQuatInterpolator();
  virtual void AddEvent(AniEventPtr event);
  virtual void AddEvent(float time, const pfQuat &value);
  virtual pfQuat Evaluate(float time);
  void    Evaluation(float time, pfQuat &q);
  bool Store(const std::string &fname) const;
  virtual void Store(FILE *f) const;
  bool Load(const std::string &fname);
  virtual void Load(FILE *f);
private:
  pfQuat EvaluateQuat(float time);
};

#endif

// $Log: quatint.hh,v $
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
// Revision 1.3  2001/02/27 10:10:30  bram
// added importer for saranav path files
//
// Revision 1.2  2001/02/22 15:47:36  bram
// Improved transform animator.
// Added serializing for ani interpolators.
//
// Revision 1.1.1.1  2000/08/23 08:02:41  bram
// Added a PyPer version to the repository
//
// Revision 1.1.1.1  2000/08/22 17:04:20  bram
// Added pyper
//

