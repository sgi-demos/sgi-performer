//
// $Source: /oss/CVS/cvs/performer/src/pyper/v3int.C,v $
// $Revision: 1.1 $
// $Author: flynnt $
// $Date: 2001/05/21 21:40:00 $
// Purpose:
//
// A pfVec3Interpolator class, the collection of events which make up a
// 3-valued object property animation (like position/size)
//
// (c) 1998 by Polar Pyramid.
//


#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
    

#include "v3int.hh"
#include "v3event.hh"

#include "hermite.hh"

//
// Constructor
//

pfVec3Interpolator::pfVec3Interpolator()
{
}

//
// Destructor
//

pfVec3Interpolator::~pfVec3Interpolator()
{
}

//
// Evaluate
//

pfVec3 pfVec3Interpolator::Evaluate(float time)
{
  return EvaluatepfVec3(GetCircularTime(time));
}

void pfVec3Interpolator::Evaluation(float time, pfVec3 &v)
{
  v = Evaluate(time);
}

//
// EvaluatepfVec3
//

pfVec3 pfVec3Interpolator::EvaluatepfVec3(float time)
{
  if (events.empty())
    return pfVec3();
  
  AniEventPtr left;
  AniEventPtr right;
  GetEventsAt(time, left, right);

  if (!left)
    return right->GetVec3();

  if (!right) // none found smaller, take found
    return left->GetVec3();

  // found 2 entries : interpolate them

  switch (interpolation_type)
  {
  case INTERPOLATE_NONE:      // no interpolation, just take 1st
    return left->GetVec3();
  case INTERPOLATE_NEAREST:   // nearest neighbour
    if (GetFractionAt(time, left, right) < 0.5) 
      return left->GetVec3();
    else
      return right->GetVec3();
  case INTERPOLATE_LINEAR:    // simple linear interpolation
  {
    float t = GetFractionAt(time, left, right);
    assert(t<=1.0);
    assert(t>=0.0);
    return (1.0f - t)*left->GetVec3() + t*right->GetVec3();
  }	       
  case INTERPOLATE_SPLINE:    // hermite spline interpolation
  {
    // if no interpolation needed avoid un-intuitive spline result!

    if (left->GetVec3() == right->GetVec3())
      return left->GetVec3();

    float her[4][3]; // set up hermite matrix

    CopyVec3ToFloats(left->GetVec3(),her[0]);
    CopyVec3ToFloats(right->GetVec3(),her[1]);
    CopyVec3ToFloats(left->GetTangentOut(),her[2]);
    CopyVec3ToFloats(right->GetTangentIn(),her[3]);

    float mat[4][3];   // hermite matrix, could be cached for optimization
    mulher(her, mat);  // make it into a hermite mat

    float t = GetFractionAt(time, left, right);
    return cubpos(mat, t);
  }
  default:
    assert(0);
    break;
  }
  return pfVec3();
}

//
// EvaluateDerivative
//

pfVec3 pfVec3Interpolator::EvaluateDerivative(float time)
{
  return EvaluateTangent(GetCircularTime(time));
}

//
// EvaluateTangent
//

pfVec3 pfVec3Interpolator::EvaluateTangent(float time)
{
  if (events.empty())
    return pfVec3();
  
  AniEventPtr left;
  AniEventPtr right;
  GetEventsAt(time, left, right);

  if (!left)
    return pfVec3();

  if (!right) // none found smaller, take found
    return pfVec3();

  // found 2 entries : interpolate them

  switch (interpolation_type)
  {
  case INTERPOLATE_NEAREST:   // nearest neighbour
  case INTERPOLATE_NONE:      // no interpolation, no or infinite derivative
    return pfVec3();
  case INTERPOLATE_LINEAR:    // simple linear interpolation: constant velocity
    return (right->GetVec3() - left->GetVec3())/(right->GetTime() - left->GetTime());
  case INTERPOLATE_SPLINE:    // hermite spline interpolation
  {
    float her[4][3]; // set up hermite matrix

    CopyVec3ToFloats(left->GetVec3(),her[0]);
    CopyVec3ToFloats(right->GetVec3(),her[1]);
    CopyVec3ToFloats(left->GetTangentOut(),her[2]);
    CopyVec3ToFloats(right->GetTangentIn(),her[3]);

    float mat[4][3];   // hermite matrix, could be cached for optimization
    mulher(her, mat);  // make it into a hermite mat

    float t = GetFractionAt(time, left, right);
    return cubtan(mat, t);
  }
  default:
    assert(0);
    break;
  }
  return pfVec3();
}


inline void ValidateIndex(int  &idx,
			  bool circular,
			  int  min_idx,
			  int  max_idx)
{
    if (idx < min_idx)
    {
      if (circular)
	idx += max_idx;
      else
	idx = min_idx;
    }
    if (idx > max_idx)
    {
      if (circular)
	idx -= max_idx;
      else
	idx = max_idx;
    }
    assert(idx >= min_idx);
    assert(idx <= max_idx);
}


//
// SetDerivatives
//

void pfVec3Interpolator::SetDerivatives(void)
{
  if (events.size() < 2)
    return;

  const int min_idx = 0;
  int max_idx = events.size() - 1;
  for (int i = 0; i <= max_idx; i++)
  {
    int prev_idx = i - 1;
    int next_idx = i + 1;
    ValidateIndex(prev_idx, circular, min_idx, max_idx);
    ValidateIndex(next_idx, circular, min_idx, max_idx);

#if 0
    pfVec3 intangent  = events[i]->GetVec3() - events[prev_idx]->GetVec3();
    pfVec3 outtangent = events[next_idx]->GetVec3() - events[i]->GetVec3();

    intangent  = derivative_scale * intangent;
    outtangent = derivative_scale * outtangent;

    events[i]->SetTangentIn(intangent);
    events[i]->SetTangentOut(outtangent);
#else
    pfVec3 tangent = events[next_idx]->GetVec3() - events[prev_idx]->GetVec3();
    events[i]->SetTangentIn(tangent);
    events[i]->SetTangentOut(tangent);
#endif
  }
}



//
// AddEvent
//

void pfVec3Interpolator::AddEvent(AniEventPtr event)
{
  AniInterpolator::AddEvent(event);
  SetDerivatives();
}

void pfVec3Interpolator::AddEvent(float time, const pfVec3 &value)
{
  AddEvent(new pfVec3Event(value, time));
}


bool pfVec3Interpolator::Store(const std::string &fname) const
{
  FILE *f = fopen(fname.c_str(), "wb");
  if (!f)
    return false;
  Store(f);
  fclose(f);
  return true;
}


void pfVec3Interpolator::Store(FILE *f) const
{
  AniInterpolator::Store(f);
  int size = events.size();
  fwrite(&size, sizeof(size), 1, f);
  for (int i=0; i<size; i++)
    events[i]->Store(f);
}


bool pfVec3Interpolator::Load(const std::string &fname)
{
  FILE *f = fopen(fname.c_str(), "rb");
  if (!f)
  {
    fprintf(stderr,"File '%s' not found\n", fname.c_str());
    return false;
  }
  Load(f);
  fclose(f);
  return true;
}


void pfVec3Interpolator::Load(FILE *f)
{
  // remove old data
  Clear();

  AniInterpolator::Load(f);
  int size;
  fread(&size, sizeof(size), 1, f);
  for (int i=0; i<size; i++)
  {
    pfVec3EventPtr ev = new pfVec3Event();
    ev->Load(f);
    AddEvent(ev);
  }
  printf("Loaded pfVec3Interpolator with %d events.\n", events.size());
}


// $Log: v3int.C,v $
// Revision 1.1  2001/05/21 21:40:00  flynnt
// Doing some cleanup and adding the pfgtk example and the python wrapper for
// Performer (pyper).
//
// Revision 1.6  2001/04/10 13:50:42  bram
// Avoided return by value of ::Evaluate()
// We now have an alternative method ::Evaluation() that passes
// the result as a function parameter instead.
// This avoids problems with swig.
//
// Revision 1.5  2001/03/19 12:32:31  bram
// Fixed spline interpolation of pfVec3
// Stamped out usage of ASSERT. Should use assert instead.
//
// Revision 1.4  2001/02/27 10:10:31  bram
// added importer for saranav path files
//
// Revision 1.3  2001/02/26 07:54:48  bram
// fixed path recording
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

