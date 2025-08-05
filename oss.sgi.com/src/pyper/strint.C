//
// $Source: /oss/CVS/cvs/performer/src/pyper/strint.C,v $
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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
    
#include "strint.hh"
#include "strevent.hh"

//
// Constructor
//

StringInterpolator::StringInterpolator()
{
}

//
// Destructor
//

StringInterpolator::~StringInterpolator()
{
}

//
// Evaluate
//

std::string StringInterpolator::Evaluate(float time)
{
  return EvaluateString(GetCircularTime(time));
}

//
// EvaluateString
//

void StringInterpolator::AddEvent(AniEventPtr event)
{
  AniInterpolator::AddEvent(event);
}

void StringInterpolator::AddEvent(float time, const std::string &value)
{
  AniInterpolator::AddEvent(new StringEvent(value, time));
}


std::string StringInterpolator::EvaluateString(float time)
{
  if (events.empty())
    return std::string();

  AniEventPtr left;
  AniEventPtr right;
  GetEventsAt(time, left, right);

  if (!left)
    return right->GetString();

  if (!right) // none found smaller, take found
    return left->GetString();

  // found 2 entries : interpolate them

  switch (interpolation_type)
  {
  case INTERPOLATE_NONE:      // no interpolation, just take 1st
    return left->GetString();
  case INTERPOLATE_LINEAR:    // simple linear interpolation
    return left->GetString();
  case INTERPOLATE_NEAREST:   // nearest neighbour
    if (GetFractionAt(time, left, right) < 0.5) 
      return left->GetString();
    else
      return right->GetString();
  default:
    assert(0);
    break;
  }
  return std::string();
}


bool StringInterpolator::Store(const std::string &fname) const
{
  FILE *f = fopen(fname.c_str(), "wb");
  if (!f)
    return false;
  Store(f);
  fclose(f);
  return true;
}


void StringInterpolator::Store(FILE *f) const
{
  AniInterpolator::Store(f);
  int size = events.size();
  fwrite(&size, sizeof(size), 1, f);
  for (int i=0; i<size; i++)
    events[i]->Store(f);
}


bool StringInterpolator::Load(const std::string &fname)
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


void StringInterpolator::Load(FILE *f)
{
  // remove old data
  Clear();

  AniInterpolator::Load(f);
  int size;
  fread(&size, sizeof(size), 1, f);
  for (int i=0; i<size; i++)
  {
    StringEventPtr ev = new StringEvent();
    ev->Load(f);
    AddEvent(ev);
  }
  printf("Loaded StringInterpolator with %d events.\n", events.size());
}




// $Log: strint.C,v $
// Revision 1.1  2001/05/21 21:40:00  flynnt
// Doing some cleanup and adding the pfgtk example and the python wrapper for
// Performer (pyper).
//
// Revision 1.3  2001/03/19 12:32:31  bram
// Fixed spline interpolation of pfVec3
// Stamped out usage of ASSERT. Should use assert instead.
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

