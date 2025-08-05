//
// $Source: /oss/CVS/cvs/performer/src/pyper/strevent.hh,v $
// $Revision: 1.1 $
// $Author: flynnt $
// $Date: 2001/05/21 21:40:00 $
// Purpose:
//
// A StringEvent class, describing string-valued events 
//
// (c) 1998 by Polar Pyramid.
//

#if !defined(STRINGEVENT_HH)
#define STRINGEVENT_HH

#include <string>
#include "anievent.hh"

//
// StringEvent
//

class StringEvent;
typedef StringEvent *StringEventPtr;

class StringEvent : public AniEvent
{
public:
  StringEvent(const std::string &ivalue = "",
	      float         itime = 0.0f);
  virtual ~StringEvent();

  virtual std::string GetString(void) const { return value; }
  virtual void Store(FILE *f) const;
  virtual void Load(FILE *f);
protected:
  std::string       value;               // the value of the event
};

#endif

// $Log: strevent.hh,v $
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
