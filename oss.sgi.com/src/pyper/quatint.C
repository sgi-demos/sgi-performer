//
// $Source: /oss/CVS/cvs/performer/src/pyper/quatint.C,v $
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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
    
#include "quatint.hh"
#include "quatevnt.hh"


//
// Constructor
//

pfQuatInterpolator::pfQuatInterpolator()
{
}

//
// Destructor
//

pfQuatInterpolator::~pfQuatInterpolator()
{
}

//
// Evaluate
//

pfQuat pfQuatInterpolator::Evaluate(float time)
{
  return EvaluateQuat(GetCircularTime(time));
}

void pfQuatInterpolator::Evaluation(float time, pfQuat &q)
{
  q = Evaluate(time);
}

//
// EvaluateQuat
//

pfQuat pfQuatInterpolator::EvaluateQuat(float time)
{
  if (events.empty())
    return pfQuat();

  AniEventPtr left;
  AniEventPtr right;
  GetEventsAt(time, left, right);

  if (!left)
    return right->GetQuat();

  if (!right) // none found smaller, take found
    return left->GetQuat();

  // found 2 entries : interpolate them

  pfQuat q;
  switch (interpolation_type)
  {
  case INTERPOLATE_NONE:      // no interpolation, just take 1st
    return left->GetQuat();
  case INTERPOLATE_NEAREST:   // nearest neighbour
    if (GetFractionAt(time, left, right) < 0.5) 
      return left->GetQuat();
    else
      return right->GetQuat();
  case INTERPOLATE_LINEAR:    // Spherical linear interpolation (SLERP)
    q.slerp(GetFractionAt(time,left,right),left->GetQuat(),right->GetQuat());
    return q;
  case INTERPOLATE_SPLINE: 
    // for quadtratic interpolation, we need 4 quaternions!
    if (1) 
    {
      AniEventPtr leftleft;
      AniEventPtr rightright;
      leftleft   = GetPredecessor(left);
      rightright = GetSuccessor(right);
#if 0
      if (leftleft && rightright)
        printf("Yeah! we can interpolate the quaterion quadratically\n");
#endif
      if (!leftleft)
        leftleft = left;
      if (!rightright)
        rightright = right;
#if 1
      q.squad
      (
        GetFractionAt(time,left,right),
        left->GetQuat(),
        right->GetQuat(),
        leftleft->GetQuat(),
        rightright->GetQuat()
      );
#else
      q.squad
      (
        GetFractionAt(time,left,right),
        leftleft->GetQuat(),
        left->GetQuat(),
        right->GetQuat(),
        rightright->GetQuat()
      );
#endif

      return q;
    }
  default:
    assert(0);
    break;
  }
  return pfQuat();
}


void pfQuatInterpolator::AddEvent(AniEventPtr event)
{
  AniInterpolator::AddEvent(event);
}


void pfQuatInterpolator::AddEvent(float time, const pfQuat &value)
{
  AniInterpolator::AddEvent(new pfQuatEvent(value, time));
}


bool pfQuatInterpolator::Store(const std::string &fname) const
{
  FILE *f = fopen(fname.c_str(), "wb");
  if (!f)
    return false;
  Store(f);
  fclose(f);
  return true;
}


void pfQuatInterpolator::Store(FILE *f) const
{
  AniInterpolator::Store(f);
  int size = events.size();
  fwrite(&size, sizeof(size), 1, f);
  for (int i=0; i<size; i++)
    events[i]->Store(f);
}


bool pfQuatInterpolator::Load(const std::string &fname)
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


void pfQuatInterpolator::Load(FILE *f)
{
  // remove old data
  Clear();

  AniInterpolator::Load(f);
  int size;
  fread(&size, sizeof(size), 1, f);
  for (int i=0; i<size; i++)
  {
    pfQuatEventPtr ev = new pfQuatEvent();
    ev->Load(f);
    AddEvent(ev);
  }
  printf("Loaded pfQuatInterpolator with %d events.\n", events.size());
}


// $Log: quatint.C,v $
// Revision 1.1  2001/05/21 21:40:00  flynnt
// Doing some cleanup and adding the pfgtk example and the python wrapper for
// Performer (pyper).
//
// Revision 1.7  2001/04/10 13:50:42  bram
// Avoided return by value of ::Evaluate()
// We now have an alternative method ::Evaluation() that passes
// the result as a function parameter instead.
// This avoids problems with swig.
//
// Revision 1.6  2001/03/19 12:32:31  bram
// Fixed spline interpolation of pfVec3
// Stamped out usage of ASSERT. Should use assert instead.
//
// Revision 1.5  2001/02/27 10:10:30  bram
// added importer for saranav path files
//
// Revision 1.4  2001/02/22 15:47:36  bram
// Improved transform animator.
// Added serializing for ani interpolators.
//
// Revision 1.3  2001/02/07 14:44:33  bram
// improved transformanimator
//
// Revision 1.2  2000/08/31 17:23:20  bram
// working anim
//
// Revision 1.1.1.1  2000/08/23 08:02:42  bram
// Added a PyPer version to the repository
//
// Revision 1.1.1.1  2000/08/22 17:04:20  bram
// Added pyper
//
