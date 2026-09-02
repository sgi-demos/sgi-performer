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

#include <Performer/pf.h>

#include <Performer/pr/pfHighlight.h>
#include <Performer/pf/pfNode.h>

#include <Performer/pfutil.h>

#include <Performer/pfv/pfvXml.h>
#include <Performer/pfv/pfvMousePicker.h>

#include "myInteractor.h"

///////////////////////////////////////////////////////////////////////////////

pfType* myInteractor::classType = NULL;

///////////////////////////////////////////////////////////////////////////////

void myInteractor::init()
{
	if(classType==NULL)
	{
		pfvInteractor::init();
		classType= new pfType(pfvInteractor::getClassType(), "myInteractor" );
	}
}

///////////////////////////////////////////////////////////////////////////////


myInteractor::myInteractor(pfvXmlNode* xml)
{
	if(classType==NULL)
		init();
	setType(classType);

	node = NULL;
	hl = new pfHighlight;
	PFASSERTALWAYS(hl);
	hl->setMode( PFHL_LINES );
 	hl->setColor( PFHL_FGCOLOR, 1.0f, 1.0f, 0.0f );

	hl_sel = new pfHighlight;
	PFASSERTALWAYS(hl_sel);
	hl_sel->setMode( PFHL_LINES );
 	hl_sel->setColor( PFHL_FGCOLOR, 0.0f, 1.0f, 0.0f );
}


///////////////////////////////////////////////////////////////////////////////

myInteractor::~myInteractor()
{
	pfDelete(hl);
}

///////////////////////////////////////////////////////////////////////////////

int myInteractor::startHlite( pfvPicker* picker, int permissions )
{
	if(node)
		pfuTravNodeHlight( node, hl );
	return (node!=NULL);
}

///////////////////////////////////////////////////////////////////////////////

void myInteractor::endHlite( pfvPicker* picker )
{
	if(node)
		pfuTravNodeHlight( node, NULL );
}

///////////////////////////////////////////////////////////////////////////////

int myInteractor::updateHlite( pfvPicker*p,int ev,int prmsn, pfvPickerRequest*r )
{
	if(ev==LEFT_DOWN)
	{
		r->state = PFPICKER_INTERACT;
		r->interactor = this;
	}
	return 1;
}

///////////////////////////////////////////////////////////////////////////////

int myInteractor::startInteraction( pfvPicker*p, int ev )
{
	if(node)
		pfuTravNodeHlight( node, hl_sel );
	
	return (node!=NULL);
}

///////////////////////////////////////////////////////////////////////////////

int myInteractor::updateInteraction( pfvPicker* p,int ev,
									 int prmsn, pfvPickerRequest*r )
{
	if( ev==LEFT_UP )
	{
		r->state = PFPICKER_ALLOW_HLITE;
	}
	return 1;
}

///////////////////////////////////////////////////////////////////////////////

void myInteractor::endInteraction( pfvPicker* p )
{
	if(node)
		pfuTravNodeHlight( node, NULL );
}

///////////////////////////////////////////////////////////////////////////////


