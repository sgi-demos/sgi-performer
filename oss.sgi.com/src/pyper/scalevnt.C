//
// $Source: /oss/CVS/cvs/performer/src/pyper/scalevnt.C,v $
// $Revision: 1.1 $
// $Author: flynnt $
// $Date: 2001/05/21 21:40:00 $
// Purpose:
//
// A ScalarEvent class, describing scalar-valued events like rotation angle
//
// (c) 1998 by Polar Pyramid.
//

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
    
#include "scalevnt.hh"

//
// Constructor
//

ScalarEvent::ScalarEvent(const float &ivalue, float itime)
  : AniEvent(itime),
    value(ivalue)
{
}

//
// Destructor
//

ScalarEvent::~ScalarEvent()
{
}


void ScalarEvent::Store(FILE *f) const
{
  AniEvent::Store(f);
  fwrite(&value, sizeof(value), 1, f);
}


void ScalarEvent::Load(FILE *f)
{
  AniEvent::Load(f);
  fread(&value, sizeof(value), 1, f);
}


// $Log: scalevnt.C,v $
// Revision 1.1  2001/05/21 21:40:00  flynnt
// Doing some cleanup and adding the pfgtk example and the python wrapper for
// Performer (pyper).
//
// Revision 1.2  2001/02/22 15:47:36  bram
// Improved transform animator.
// Added serializing for ani interpolators.
//
// Revision 1.1.1.1  2000/08/23 08:02:42  bram
// Added a PyPer version to the repository
//
// Revision 1.1.1.1  2000/08/22 17:04:20  bram
// Added pyper
//

