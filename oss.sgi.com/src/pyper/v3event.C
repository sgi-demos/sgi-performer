//
// $Source: /oss/CVS/cvs/performer/src/pyper/v3event.C,v $
// $Revision: 1.1 $
// $Author: flynnt $
// $Date: 2001/05/21 21:40:00 $
// Purpose:
//
// A pfVec3Event class, describing 3-valued events like position, scale
//
// (c) 1998 by Polar Pyramid.
//

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
    
#include "v3event.hh"

//
// Constructor
//

pfVec3Event::pfVec3Event(const pfVec3 &ivalue,
			   float         itime,
			   const pfVec3 &itangent_in,
			   const pfVec3 &itangent_out)
  : AniEvent(itime),
    value(ivalue),
    tangent_in(itangent_in),
    tangent_out(itangent_out)
{
}

//
// Destructor
//

pfVec3Event::~pfVec3Event()
{
}


void pfVec3Event::Store(FILE *f) const
{
  AniEvent::Store(f);
  fwrite(&value, sizeof(value), 1, f);
  fwrite(&tangent_in, sizeof(tangent_in), 1, f);
  fwrite(&tangent_out, sizeof(tangent_out), 1, f);
}


void pfVec3Event::Load(FILE *f)
{
  AniEvent::Load(f);
  fread(&value, sizeof(value), 1, f);
  fread(&tangent_in, sizeof(tangent_in), 1, f);
  fread(&tangent_out, sizeof(tangent_out), 1, f);
}


// $Log: v3event.C,v $
// Revision 1.1  2001/05/21 21:40:00  flynnt
// Doing some cleanup and adding the pfgtk example and the python wrapper for
// Performer (pyper).
//
// Revision 1.2  2001/02/22 15:47:37  bram
// Improved transform animator.
// Added serializing for ani interpolators.
//
// Revision 1.1.1.1  2000/08/23 08:02:41  bram
// Added a PyPer version to the repository
//
// Revision 1.1.1.1  2000/08/22 17:04:20  bram
// Added pyper
//

