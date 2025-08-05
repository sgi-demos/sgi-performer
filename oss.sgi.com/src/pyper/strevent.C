//
// $Source: /oss/CVS/cvs/performer/src/pyper/strevent.C,v $
// $Revision: 1.1 $
// $Author: flynnt $
// $Date: 2001/05/21 21:40:00 $
// Purpose:
//
// A StringEvent class, describing string-valued events 
//
// (c) 1998 by Polar Pyramid.
//

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
    
#include "strevent.hh"

//
// Constructor
//

StringEvent::StringEvent(const std::string &ivalue,
			 float         itime)
  : AniEvent(itime),
    value(ivalue)
{
}

//
// Destructor
//

StringEvent::~StringEvent()
{
}


void StringEvent::Store(FILE *f) const
{
  AniEvent::Store(f);
  const char *s = value.c_str();
  fwrite(s, strlen(s)+1, 1, f);
}


void StringEvent::Load(FILE *f)
{
  AniEvent::Load(f);
  value = "";

  bool quit=false;
  while (!quit)
  {
    int retval = fgetc(f);
    assert(retval != EOF);
    unsigned char c = (unsigned char) retval;
    char addition[2] = {c, 0};
    value = value + addition;
    if (c == 0)
      quit=true;
  }
}


// $Log: strevent.C,v $
// Revision 1.1  2001/05/21 21:40:00  flynnt
// Doing some cleanup and adding the pfgtk example and the python wrapper for
// Performer (pyper).
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

