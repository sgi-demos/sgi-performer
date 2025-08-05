//
// $Source: /oss/CVS/cvs/performer/src/pyper/anievent.hh,v $
// $Revision: 1.1 $
// $Author: flynnt $
// $Date: 2001/05/21 21:40:00 $
// Purpose:
//
// A AniEvent class, the base class of events
//
// (c) 1998 by Polar Pyramid.
//

#if !defined(ANIEVENT_HH)
#define ANIEVENT_HH

#include <string>
#include <Performer/pf.h>

#include "prt.h"

//
// AniEvent
//

class AniEvent;
typedef AniEvent *AniEventPtr;

class AniEvent
{
  public:

  AniEvent(float itime = 0.0);
  virtual ~AniEvent();

  float GetTime(void) const { return time; }
  virtual std::string GetString(void) const { return ""; }
  virtual float GetX(void) const { return 0.0f; }
  virtual float GetY(void) const { return 0.0f; }
  virtual float GetZ(void) const { return 0.0f; }
  virtual float GetW(void) const { return 0.0f; }
  virtual pfQuat GetQuat(void) const { return pfQuat(); }
  virtual float GetScalar(void) const { return 0.0f; }

  virtual const pfVec3 &GetVec3(void) const { static pfVec3 x; return x; }
  virtual const pfVec3 &GetTangentIn(void) const { static pfVec3 x; return x; }
  virtual const pfVec3 &GetTangentOut(void) const { static pfVec3 x; return x; }
  virtual void SetTangentIn(const pfVec3 &t) { }
  virtual void SetTangentOut(const pfVec3 &t) { }

  virtual void Store(FILE *f) const;
  virtual void Load(FILE *f);

  protected:

  float             time;               // the time of the event
};

inline bool LessThan(const AniEventPtr &a,
		     const AniEventPtr &b)
{
  return (a->GetTime() < b->GetTime());
}

#endif

// $Log: anievent.hh,v $
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

