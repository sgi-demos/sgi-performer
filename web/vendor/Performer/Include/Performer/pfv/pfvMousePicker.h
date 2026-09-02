//
// Copyright 2001, 2002 Silicon Graphics, Inc.
// ALL RIGHTS RESERVED
//
// UNPUBLISHED -- Rights reserved under the copyright laws of the United
// States.   Use of a copyright notice is precautionary only and does not
// imply publication or disclosure.
//
// U.S. GOVERNMENT RESTRICTED RIGHTS LEGEND:
// Use, duplication or disclosure by the Government is subject to restrictions
// as set forth in FAR 52.227.19(c)(2) or subparagraph (c)(1)(ii) of the Rights
// in Technical Data and Computer Software clause at DFARS 252.227-7013 and/or
// in similar or successor clauses in the FAR, or the DOD or NASA FAR
// Supplement.  Contractor/manufacturer is Silicon Graphics, Inc.,
// 2011 N. Shoreline Blvd. Mountain View, CA 94039-7311.
//
// THE CONTENT OF THIS WORK CONTAINS CONFIDENTIAL AND PROPRIETARY
// INFORMATION OF SILICON GRAPHICS, INC. ANY DUPLICATION, MODIFICATION,
// DISTRIBUTION, OR DISCLOSURE IN ANY FORM, IN WHOLE, OR IN PART, IS STRICTLY
// PROHIBITED WITHOUT THE PRIOR EXPRESS WRITTEN PERMISSION OF SILICON
// GRAPHICS, INC.
//
// pfvMousePicker.h
//
// $Revision: 1.4 $
// $Date: 2002/12/08 02:30:38 $
//

#ifndef __PFV_MOUDEPICKER_H__
#define __PFV_MOUSEPICKER_H__

///////////////////////////////////////////////////////////////////////////////

#include <Performer/pfutil.h>
#include <Performer/pfv/pfvPicker.h>

///////////////////////////////////////////////////////////////////////////////

class pfChannel;

///////////////////////////////////////////////////////////////////////////////


#define LEFT_DOWN 	PFUDEV_MOUSE_LEFT_DOWN
#define MIDDLE_DOWN PFUDEV_MOUSE_MIDDLE_DOWN
#define RIGHT_DOWN 	PFUDEV_MOUSE_RIGHT_DOWN
#define LEFT_UP 	(LEFT_DOWN | 8)
#define MIDDLE_UP 	(MIDDLE_DOWN | 8)
#define RIGHT_UP	(RIGHT_DOWN | 8)

///////////////////////////////////////////////////////////////////////////////

//class pfvInteractor;

///////////////////////////////////////////////////////////////////////////////


class PFV_DLLEXPORT pfvMousePicker : public pfvPicker
{
public:	
	pfvMousePicker(pfvXmlNode* xml=NULL);
	~pfvMousePicker();
	
	pfvInteractor* pick();
	int collectEvents();
	int computeIsectSeg();

	pfuMouse* getMouse() { return &mouse; }
	pfuEventStream* getEventStream() { return &events; }

private:
	pfuMouse mouse;
	pfuEventStream events;	
};


///////////////////////////////////////////////////////////////////////////////


#endif // end of __PFV_MOUSEPICKER_H__


