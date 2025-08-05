//
// $Source: /oss/CVS/cvs/performer/src/pyper/scalint.C,v $
// $Revision: 1.1 $
// $Author: flynnt $
// $Date: 2001/05/21 21:40:00 $
// Purpose:
//
// A ScalarInterpolator class, the collection of events which make up a
// scalar.
//
// (c) 1998 by Polar Pyramid.
//

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
    
#include "scalint.hh"
#include "scalevnt.hh"

#include "hermite.hh"

//
// Constructor
//

ScalarInterpolator::ScalarInterpolator()
{
}

//
// Destructor
//

ScalarInterpolator::~ScalarInterpolator()
{
}

//
// Evaluate
//

float ScalarInterpolator::Evaluate(float time)
{
  return EvaluateScalar(GetCircularTime(time));
}

//
// EvaluateScalar
//

float ScalarInterpolator::EvaluateScalar(float time)
{
  if (events.empty())
    return 0.0;
  
  AniEventPtr left;
  AniEventPtr right;
  GetEventsAt(time, left, right);

  if (!left)
    return right->GetScalar();

  if (!right) // none found smaller, take found
    return left->GetScalar();

  // found 2 entries : interpolate them

  switch (interpolation_type)
  {
    case INTERPOLATE_NONE:      // no interpolation, just take 1st
      return left->GetScalar();
    case INTERPOLATE_NEAREST:   // nearest neighbour
      if (GetFractionAt(time, left, right) < 0.5) 
        return left->GetScalar();
      else
        return right->GetScalar();
    case INTERPOLATE_LINEAR:    // simple linear interpolation
    {
      float t = GetFractionAt(time, left, right);
      return (1.0f - t)*left->GetScalar() + t*right->GetScalar();
    }	       
    case INTERPOLATE_SPLINE:    // hermite spline interpolation
    default:
      assert(0);
    break;
  }
  return 0.0;
}

//
// AddEvent
//

void ScalarInterpolator::AddEvent(AniEventPtr event)
{
  AniInterpolator::AddEvent(event);
}

void ScalarInterpolator::AddEvent(float time, const float value)
{
  AddEvent(new ScalarEvent(value, time));
}



bool ScalarInterpolator::Store(const std::string &fname) const
{
  FILE *f = fopen(fname.c_str(), "wb");
  if (!f)
    return false;
  Store(f);
  fclose(f);
  return true;
}


void ScalarInterpolator::Store(FILE *f) const
{
  AniInterpolator::Store(f);
  int size = events.size();
  fwrite(&size, sizeof(size), 1, f);
  for (int i=0; i<size; i++)
    events[i]->Store(f);
}


bool ScalarInterpolator::Load(const std::string &fname)
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


void ScalarInterpolator::Load(FILE *f)
{
  // remove old data
  Clear();

  AniInterpolator::Load(f);
  int size;
  fread(&size, sizeof(size), 1, f);
  for (int i=0; i<size; i++)
  {
    ScalarEventPtr ev = new ScalarEvent();
    ev->Load(f);
    AddEvent(ev);
  }
  printf("Loaded ScalarInterpolator with %d events.\n", events.size());
}



// $Log: scalint.C,v $
// Revision 1.1  2001/05/21 21:40:00  flynnt
// Doing some cleanup and adding the pfgtk example and the python wrapper for
// Performer (pyper).
//
// Revision 1.5  2001/03/19 12:32:31  bram
// Fixed spline interpolation of pfVec3
// Stamped out usage of ASSERT. Should use assert instead.
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
// Revision 1.1.1.1  2000/08/23 08:02:42  bram
// Added a PyPer version to the repository
//
// Revision 1.1.1.1  2000/08/22 17:04:20  bram
// Added pyper
//

