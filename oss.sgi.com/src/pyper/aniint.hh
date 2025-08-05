//
// $Source: /oss/CVS/cvs/performer/src/pyper/aniint.hh,v $
// $Revision: 1.1 $
// $Author: flynnt $
// $Date: 2001/05/21 21:40:00 $
// Purpose:
//
// An AniInterpolator class, the collection of events which make up an
// object property animation
// All editing is done on this class for each event type we can write our own 
// derived interpolator
//
// (c) 1998 by Polar Pyramid.
//

#if !defined(ANIINTERPOLATOR_HH)
#define ANIINTERPOLATOR_HH

#include <vector>

#include "anievent.hh"


inline void CopyVec3ToFloats(const pfVec3 &v, float *f)
{
  f[0] = v[0];
  f[1] = v[1];
  f[2] = v[2];
}


enum InterpolationType
{
  INTERPOLATE_NONE,      // no interpolation
  INTERPOLATE_NEAREST,   // nearest neighbour
  INTERPOLATE_LINEAR,    // linear interpolation
  INTERPOLATE_SPLINE     // spline type interpolation
};


//
// AniInterpolator
//

class AniInterpolator;
typedef AniInterpolator *AniInterpolatorPtr;

class AniInterpolator 
{
public:
  AniInterpolator();
  virtual ~AniInterpolator();

  virtual void AddEvent(AniEventPtr event);
#if 0
  virtual GenericType Evaluate(float time);
  virtual GenericType EvaluateDerivative(float time);
#endif

  bool GetNextFix(float &time);
  void DeleteFix(float time);
  void GetEventsAt(float        time,
		   AniEventPtr &left,
		   AniEventPtr &right);
  inline float GetFractionAt(float              time,
			     const AniEventPtr &left,
			     const AniEventPtr &right);
  float GetMinTime(void) const;
  float GetMaxTime(void) const;
  void SetCircular(bool c) { circular = c; }
  void SetInterpolationType(InterpolationType i) { interpolation_type = i; }
  void SetDerivativeScale(float scale) { derivative_scale = scale; }
  AniEventPtr GetPredecessor(AniEventPtr &p) const;
  AniEventPtr GetSuccessor(const AniEventPtr &p) const;
  float GetClosestKeyTime(float t) const;
  int   GetEventCount(void) const { return events.size(); }
  float GetTimeAtEvent(int nr) const;
  virtual void Store(FILE *f) const;
  virtual void Load(FILE *f);
  void Clear(void);

protected:
  float GetCircularTime(float time) const;

  bool                      circular;         // circular (repeating)
  InterpolationType         interpolation_type;// type of interpolation
  float                     derivative_scale; // for scaling automatic derivative (tangents) 
  std::vector<AniEventPtr>  events;           // a list of all events in time
};

//
// AlmostEqual
//

inline bool AlmostEqual(float t0, float t1)
{
  return (fabs(t0 - t1) < 0.01f);
}

//
// GetFractionAt
//

inline float AniInterpolator::GetFractionAt(float              time,
					    const AniEventPtr &left,
					    const AniEventPtr &right)
{
  float tleft = left->GetTime();
  float tright = right->GetTime();

  return (time - tleft)/(tright - tleft);
}


#endif

// $Log: aniint.hh,v $
// Revision 1.1  2001/05/21 21:40:00  flynnt
// Doing some cleanup and adding the pfgtk example and the python wrapper for
// Performer (pyper).
//
// Revision 1.7  2001/03/19 12:32:31  bram
// Fixed spline interpolation of pfVec3
// Stamped out usage of ASSERT. Should use assert instead.
//
// Revision 1.6  2001/02/27 16:09:43  bram
// Added serailizing for string interpolators
//
// Revision 1.5  2001/02/26 07:54:47  bram
// fixed path recording
//
// Revision 1.4  2001/02/22 15:47:36  bram
// Improved transform animator.
// Added serializing for ani interpolators.
//
// Revision 1.3  2000/11/21 13:05:21  bram
// improved transform animator
//
// Revision 1.2  2000/11/16 13:51:56  bram
// added transform animator
//
// Revision 1.1.1.1  2000/08/23 08:02:42  bram
// Added a PyPer version to the repository
//
// Revision 1.1.1.1  2000/08/22 17:04:20  bram
// Added pyper
//

