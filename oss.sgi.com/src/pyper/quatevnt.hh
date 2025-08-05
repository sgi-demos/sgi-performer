//
// $Source: /oss/CVS/cvs/performer/src/pyper/quatevnt.hh,v $
// $Revision: 1.1 $
// $Author: flynnt $
// $Date: 2001/05/21 21:40:00 $
// Purpose:
//
// A QuatEvent class, describing quaternion-valued events like rotation
//
// (c) 1998 by Polar Pyramid.
//

#if !defined(QUATEVENT_HH)
#define QUATEVENT_HH

#include "anievent.hh"

//
// QuatEvent
//

class pfQuatEvent;
typedef pfQuatEvent *pfQuatEventPtr;

class pfQuatEvent : public AniEvent
{
public:
  pfQuatEvent(const pfQuat  &ivalue = pfQuat(),
	    float        itime = 0.0);

  virtual ~pfQuatEvent();

  virtual float GetX(void) const 
  {
    return value[0];
  }
  virtual float GetY(void) const 
  {
    return value[1];
  }
  virtual float GetZ(void) const 
  {
    return value[2];
  }
  virtual float GetW(void) const 
  {
    return value[3];
  }
  virtual pfQuat GetQuat(void) const 
  {
    return value; 
  }
#if 0
  virtual const pfVec3 &GetVec3(void) const 
  {
    return pfVec3(value[0],value[1],value[2]);
  }
#endif
  virtual void Store(FILE *f) const;
  virtual void Load(FILE *f);

protected:
  pfQuat           value;               // the value of the event
};

#endif

// $Log: quatevnt.hh,v $
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

