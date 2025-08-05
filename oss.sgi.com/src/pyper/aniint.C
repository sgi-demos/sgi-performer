//
// $Source: /oss/CVS/cvs/performer/src/pyper/aniint.C,v $
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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
    
#include "aniint.hh"

#include <algorithm>

//
// Constructor
//

AniInterpolator::AniInterpolator()
  : circular(true),
    interpolation_type(INTERPOLATE_LINEAR),
    derivative_scale(1.0f)
{
}

//
// Destructor
//

AniInterpolator::~AniInterpolator()
{
  Clear();
}

//
// Clear
//

void AniInterpolator::Clear(void)
{
  events.clear();
}

//
// AddEvent
//

void AniInterpolator::AddEvent(AniEventPtr event)
{
  DeleteFix(event->GetTime()); // overwrite any event at time

  events.push_back(event); 

  std::sort(events.begin(), events.end(), LessThan);
}

//
// GetNextFix
//

bool AniInterpolator::GetNextFix(float &time)
{
  std::vector<AniEventPtr>::iterator ip = events.begin();
  while (ip != events.end()) 
  {
    if ((*ip)->GetTime() > time)
    {
      time = (*ip)->GetTime();
      return true;
    }
    ++ip;
  }
  return false;
}

//
// DeleteFix
//

void AniInterpolator::DeleteFix(float time)
{
  std::vector<AniEventPtr>::iterator ip = events.begin();
  while (ip != events.end()) 
  {
    if (AlmostEqual((*ip)->GetTime(), time))
    {
      ip = events.erase(ip);
      return;
    }
    ++ip;
  }
}

//
// GetEventsAt
//  get the 2 events surrounding the given time
//

void AniInterpolator::GetEventsAt(float        time,
				  AniEventPtr &left,
				  AniEventPtr &right)
{
  // find 1st event with time bigger than given time

  if (events.empty())
  {
    left = right = 0;
    return;
  }

  std::vector<AniEventPtr>::iterator i;
  for (i = events.begin(); i != events.end(); ++i)
  {
    if ((*i)->GetTime() >= time) // TODO, not optimal if many events
      break;
  }
  if (i == events.end()) // none found bigger, take last
  {
    --i;
    left = 0;
    right = (*i);
  }
  else // found bigger: i
  {
    if (i == events.begin()) // none found smaller, take found
    {
      left = (*i);
      right = 0;
    }
    else // found 2 entries : i & j
    {
      std::vector<AniEventPtr>::iterator j = i;
      --i;
      left = (*i);
      right = (*j);
    }
  }
}

//
// GetMinTime
//

float AniInterpolator::GetMinTime(void) const
{
  if (events.empty())
    return 0.0f;

  return events.front()->GetTime();
}

//
// GetMaxTime
//

float AniInterpolator::GetMaxTime(void) const
{
  if (events.empty())
    return 0.0f;

  return events.back()->GetTime();
}

//
// GetCircularTime
//  if circular, reposition time between min & max
//

float AniInterpolator::GetCircularTime(float time) const
{
  if (!circular)
    return time;

  float min_time = GetMinTime();
  float max_time = GetMaxTime();

  if (time < min_time)
    return min_time;

  if (time > max_time)
  {
    float total_time = max_time - min_time;
    if (total_time == 0.0)
      return min_time;
    float val = (time - min_time)/total_time;
    int ival = static_cast<int>(val);
    return min_time + (val - ival)*total_time;
  }
  return time;
}

AniEventPtr AniInterpolator::GetPredecessor(AniEventPtr &p) const
{
  for (unsigned int i = 0; i<events.size(); i++)
    if (events[i] == p)
      if (i>0)
        return events[i-1];
  return 0;
}

AniEventPtr AniInterpolator::GetSuccessor(const AniEventPtr &p) const
{
  for (unsigned int i = 0; i<events.size(); i++)
    if (events[i] == p)
      if (i<events.size()-1)
        return events[i+1];
  return 0;
}

float AniInterpolator::GetClosestKeyTime(float t) const
{
   t = GetCircularTime(t);
   float best_key = 0;
   float best_delta = 0;
   for (unsigned int i = 0; i<events.size(); i++)
   {
     float delta = fabs(events[i]->GetTime() - t);
     if (i==0 || delta < best_delta)
     {
       best_key = events[i]->GetTime();
       best_delta = delta;
     }
   }
   return best_key;
}

float AniInterpolator::GetTimeAtEvent(int nr) const
{
  assert(nr >= 0);
  assert(nr < events.size());
  return events[nr]->GetTime();
}


void AniInterpolator::Store(FILE *f) const
{
  fwrite(&circular, sizeof(circular), 1, f);
  fwrite(&interpolation_type, sizeof(interpolation_type), 1, f);
  fwrite(&derivative_scale, sizeof(derivative_scale), 1, f);
}


void AniInterpolator::Load(FILE *f)
{
  fread(&circular, sizeof(circular), 1, f);
  fread(&interpolation_type, sizeof(interpolation_type), 1, f);
  fread(&derivative_scale, sizeof(derivative_scale), 1, f);
}


// $Log: aniint.C,v $
// Revision 1.1  2001/05/21 21:40:00  flynnt
// Doing some cleanup and adding the pfgtk example and the python wrapper for
// Performer (pyper).
//
// Revision 1.6  2001/03/19 12:32:30  bram
// Fixed spline interpolation of pfVec3
// Stamped out usage of ASSERT. Should use assert instead.
//
// Revision 1.5  2001/02/22 15:47:36  bram
// Improved transform animator.
// Added serializing for ani interpolators.
//
// Revision 1.4  2000/11/21 13:05:21  bram
// improved transform animator
//
// Revision 1.3  2000/11/16 13:51:56  bram
// added transform animator
//
// Revision 1.2  2000/08/23 14:17:09  bram
// fixed warning
//
// Revision 1.1.1.1  2000/08/23 08:02:42  bram
// Added a PyPer version to the repository
//
// Revision 1.1.1.1  2000/08/22 17:04:20  bram
// Added pyper
//
