//
// $Source: /oss/CVS/cvs/performer/src/pyper/scalevnt.hh,v $
// $Revision: 1.1 $
// $Author: flynnt $
// $Date: 2001/05/21 21:40:00 $
// Purpose:
//
// A ScalarEvent class, describing scalar-valued events like rotation angle
//
// (c) 1998 by Polar Pyramid.

#if !defined(SCALAREVENT_HH)
#define SCALAREVENT_HH

#include "anievent.hh"

//
// ScalarEvent
//

class ScalarEvent;
typedef ScalarEvent *ScalarEventPtr;

class ScalarEvent : public AniEvent
{
public:
  ScalarEvent(const float &ivalue = 0.0, float itime = 0.0);
  virtual ~ScalarEvent();
  virtual float GetScalar(void) const { return value; }
  virtual void Store(FILE *f) const;
  virtual void Load(FILE *f);
protected:
  float value;               // the value of the event
};

#endif

// $Log: scalevnt.hh,v $
// Revision 1.1  2001/05/21 21:40:00  flynnt
// Doing some cleanup and adding the pfgtk example and the python wrapper for
// Performer (pyper).
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
