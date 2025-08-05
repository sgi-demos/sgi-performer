//
// $Source: /oss/CVS/cvs/performer/src/pyper/v3event.hh,v $
// $Revision: 1.1 $
// $Author: flynnt $
// $Date: 2001/05/21 21:40:00 $
// Purpose:
//
// A pfVec3Event class, describing 3-valued events like position, scale
//
// (c) 1998 by Polar Pyramid.
//

#if !defined(VECTOR3EVENT_HH)
#define VECTOR3EVENT_HH

#include "anievent.hh"

//
// pfVec3Event
//

class pfVec3Event;
typedef pfVec3Event *pfVec3EventPtr;

class pfVec3Event : public AniEvent
{
public:
  pfVec3Event(const pfVec3 &ivalue = pfVec3(),
	       float         itime = 0.0f,
	       const pfVec3 &itangent_in = pfVec3(),
	       const pfVec3 &itangent_out = pfVec3());
  virtual ~pfVec3Event();

  virtual float GetX(void) const { return value[0]; }
  virtual float GetY(void) const { return value[1]; }
  virtual float GetZ(void) const { return value[2]; }
  virtual const pfVec3 &GetVec3(void) const { return value; }
  virtual const pfVec3 &GetTangentIn(void) const { return tangent_in; }
  virtual const pfVec3 &GetTangentOut(void) const { return tangent_out; }
  void SetTangentIn(const pfVec3 &t) { tangent_in = t; }
  void SetTangentOut(const pfVec3 &t) { tangent_out = t; }
  virtual void Store(FILE *f) const;
  virtual void Load(FILE *f);
protected:
  pfVec3       value;               // the value of the event
  pfVec3       tangent_in;          // the incoming derivative
  pfVec3       tangent_out;         // the outgoing derivative
};

#endif

// $Log: v3event.hh,v $
// Revision 1.1  2001/05/21 21:40:00  flynnt
// Doing some cleanup and adding the pfgtk example and the python wrapper for
// Performer (pyper).
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
