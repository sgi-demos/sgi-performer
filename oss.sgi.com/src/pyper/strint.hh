//
// $Source: /oss/CVS/cvs/performer/src/pyper/strint.hh,v $
// $Revision: 1.1 $
// $Author: flynnt $
// $Date: 2001/05/21 21:40:00 $
// Purpose:
//
// A StringInterpolator class, the collection of events which make up a
// string-valued object property animation (like modelname)
//
// (c) 1998 by Polar Pyramid.
//


#if !defined(STRINGINTERPOLATOR_HH)
#define STRINGINTERPOLATOR_HH

#include "aniint.hh"

//
// StringInterpolator
//

class StringInterpolator;
typedef class StringInterpolator *StringInterpolatorPtr;

class StringInterpolator : public AniInterpolator
{
public:
  StringInterpolator();
  virtual ~StringInterpolator();

  virtual void AddEvent(AniEventPtr event);
  virtual void AddEvent(float time, const std::string &value);

  virtual std::string Evaluate(float time);
  bool Store(const std::string &fname) const;
  virtual void Store(FILE *f) const;
  bool Load(const std::string &fname);
  virtual void Load(FILE *f);

private:
  std::string EvaluateString(float time);
};

#endif

// $Log: strint.hh,v $
// Revision 1.1  2001/05/21 21:40:00  flynnt
// Doing some cleanup and adding the pfgtk example and the python wrapper for
// Performer (pyper).
//
// Revision 1.2  2001/02/27 16:09:43  bram
// Added serailizing for string interpolators
//
// Revision 1.1.1.1  2000/08/23 08:02:42  bram
// Added a PyPer version to the repository
//
// Revision 1.1.1.1  2000/08/22 17:04:20  bram
// Added pyper
//

