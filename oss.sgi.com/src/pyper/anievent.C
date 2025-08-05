//
// $Source: /oss/CVS/cvs/performer/src/pyper/anievent.C,v $
// $Revision: 1.1 $
// $Author: flynnt $
// $Date: 2001/05/21 21:40:00 $
// Purpose:
//
// A AniEvent class, the base class of events
//
// (c) 1998 by Polar Pyramid.
//

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
    
#include "anievent.hh"

//
// Constructor
//

AniEvent::AniEvent(float itime)
  : time(itime)
{
}

//
// Destructor
//

AniEvent::~AniEvent()
{
}


void AniEvent::Store(FILE *f) const
{
  fwrite(&time, sizeof(time), 1, f);
}


void AniEvent::Load(FILE *f)
{
  fread(&time, sizeof(time), 1, f);
}



// $Log: anievent.C,v $
// Revision 1.1  2001/05/21 21:40:00  flynnt
// Doing some cleanup and adding the pfgtk example and the python wrapper for
// Performer (pyper).
//
// Revision 1.2  2001/02/22 15:47:35  bram
// Improved transform animator.
// Added serializing for ani interpolators.
//
// Revision 1.1.1.1  2000/08/23 08:02:42  bram
// Added a PyPer version to the repository
//
// Revision 1.1.1.1  2000/08/22 17:04:20  bram
// Added pyper
//

