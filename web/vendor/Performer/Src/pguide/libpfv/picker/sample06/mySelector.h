//
// Copyright 2002, Silicon Graphics, Inc.
// ALL RIGHTS RESERVED
//
// This source code ("Source Code") was originally derived from a
// code base owned by Silicon Graphics, Inc. ("SGI")
// 
// LICENSE: SGI grants the user ("Licensee") permission to reproduce,
// distribute, and create derivative works from this Source Code,
// provided that: (1) the user reproduces this entire notice within
// both source and binary format redistributions and any accompanying
// materials such as documentation in printed or electronic format;
// (2) the Source Code is not to be used, or ported or modified for
// use, except in conjunction with OpenGL Performer; and (3) the
// names of Silicon Graphics, Inc.  and SGI may not be used in any
// advertising or publicity relating to the Source Code without the
// prior written permission of SGI.  No further license or permission
// may be inferred or deemed or construed to exist with regard to the
// Source Code or the code base of which it forms a part. All rights
// not expressly granted are reserved.
// 
// This Source Code is provided to Licensee AS IS, without any
// warranty of any kind, either express, implied, or statutory,
// including, but not limited to, any warranty that the Source Code
// will conform to specifications, any implied warranties of
// merchantability, fitness for a particular purpose, and freedom
// from infringement, and any warranty that the documentation will
// conform to the program, or any warranty that the Source Code will
// be error free.
// 
// IN NO EVENT WILL SGI BE LIABLE FOR ANY DAMAGES, INCLUDING, BUT NOT
// LIMITED TO DIRECT, INDIRECT, SPECIAL OR CONSEQUENTIAL DAMAGES,
// ARISING OUT OF, RESULTING FROM, OR IN ANY WAY CONNECTED WITH THE
// SOURCE CODE, WHETHER OR NOT BASED UPON WARRANTY, CONTRACT, TORT OR
// OTHERWISE, WHETHER OR NOT INJURY WAS SUSTAINED BY PERSONS OR
// PROPERTY OR OTHERWISE, AND WHETHER OR NOT LOSS WAS SUSTAINED FROM,
// OR AROSE OUT OF USE OR RESULTS FROM USE OF, OR LACK OF ABILITY TO
// USE, THE SOURCE CODE.
// 
// Contact information:  Silicon Graphics, Inc., 
// 1600 Amphitheatre Pkwy, Mountain View, CA  94043, 
// or:  http://www.sgi.com
//
//

///////////////////////////////////////////////////////////////////////////////

#ifndef __MY_SELECTOR_H__
#define __MY_SELECTOR_H__

///////////////////////////////////////////////////////////////////////////////

#include <Performer/pf/pfNode.h>
#include <Performer/pr/pfHighlight.h>

#include <Performer/pfv/pfvPicker.h>

///////////////////////////////////////////////////////////////////////////////

class pfvXmlNode;
class pfNode;

///////////////////////////////////////////////////////////////////////////////

class mySelector : public pfvSelector
{
public:
	mySelector( pfvXmlNode* xml=NULL );
	~mySelector();

	static void init();
	static pfType* getClassType(){ return classType; }
	virtual const char* getTypeName() const {return "mySelector";}

	int startHlite( pfvPicker*p, int prmsn );
	int updateHlite( pfvPicker* p,int ev,int prmsn, pfvPickerRequest*r );
	void endHlite( pfvPicker* p );

	int startSelection( pfvPicker* p, int event );
	int updateSelection( pfvPicker* p, int event, pfvPickerRequest* req );
	void endSelection( pfvPicker* p );

	int startManip( pfvPicker* p, int event );
	int updateManip( pfvPicker* p,int event, pfvPickerRequest* req );

	
	static pfType *classType;
	
	pfHighlight *hl;
	pfHighlight *hl_manip;
	pfHighlight *hl_sel;
		
	pfNode* node;
	float scale;
};

///////////////////////////////////////////////////////////////////////////////

#endif // end of __MY_SELECTOR_H__






