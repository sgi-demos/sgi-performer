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
#include <Performer/pf/pfDCS.h>

#include <Performer/pfutil.h>

#include <Performer/pfv/pfvXml.h>
#include <Performer/pfv/pfvMousePicker.h>

#include "mySelector.h"

///////////////////////////////////////////////////////////////////////////////

pfType* mySelector::classType = NULL;

///////////////////////////////////////////////////////////////////////////////

void mySelector::init()
{
	if(classType==NULL)
	{
		pfvSelector::init();
		classType= new pfType(pfvSelector::getClassType(), "mySelector" );
	}
}

///////////////////////////////////////////////////////////////////////////////


mySelector::mySelector(pfvXmlNode* xml)
{
	if(classType==NULL)
		init();
	setType(classType);

	node = NULL;
	
	hl_manip = new pfHighlight;
	PFASSERTALWAYS(hl_manip);
	hl_manip->setMode( PFHL_LINES );
 	hl_manip->setColor( PFHL_FGCOLOR, 0.3f, 1.0f, 1.0f );

	hl_sel = new pfHighlight;
	PFASSERTALWAYS(hl_sel);
	hl_sel->setMode( PFHL_LINES );
 	hl_sel->setColor( PFHL_FGCOLOR, 0.15f, 0.5f, 0.5f );
}


///////////////////////////////////////////////////////////////////////////////

mySelector::~mySelector()
{
	pfDelete(hl_sel);
	pfDelete(hl_manip);
}

///////////////////////////////////////////////////////////////////////////////

int mySelector::specialFocus( pfvPicker* p,int ev,int prmsn,pfvPickerRequest*r )
{
	if((ev==LEFT_DOWN) || (ev==RIGHT_DOWN) )
	{
		r->state = PFPICKER_SELECT | PFPICKER_FOCUS_EVENT;
		r->selector = this;
	}
	return 1;
}


///////////////////////////////////////////////////////////////////////////////

int mySelector::startManip( pfvPicker*p, int ev )
{
	if(node)
	{
		pfuTravNodeHlight( node, hl_manip );

		if(ev==LEFT_DOWN)
			scale = 1.01f;
		else
			scale = 1.0f/1.01f;
	}
	return (node!=NULL);
}

///////////////////////////////////////////////////////////////////////////////

int mySelector::updateManip( pfvPicker* p,int ev, pfvPickerRequest*r )
{
	if( ((scale>1.0f)&&(ev==LEFT_UP)) || (ev==RIGHT_UP) )
	{
		r->state = PFPICKER_SELECT | PFPICKER_FOCUS_EVENT;
		r->selector = this;
		pfuTravNodeHlight( node, hl_sel );
	}
	else		
	{
		pfMatrix m;
		pfDCS* dcs = (pfDCS*)node;
		
		dcs->getMat(m);
		pfMatrix scalemat;
		scalemat.makeScale(scale,scale,scale);
		m.preMult(scalemat);
		dcs->setMat(m);
	}
	
	return 1;
}

///////////////////////////////////////////////////////////////////////////////

int mySelector::startSelection( pfvPicker*p, int ev )
{
	if(node)
	{
		pfuTravNodeHlight( node, hl_sel );
		
		if(ev==LEFT_DOWN)
			scale = 1.01f;
		else
			scale = 1.0f/1.01f;
	}
	
	return (node!=NULL);

}

///////////////////////////////////////////////////////////////////////////////


int mySelector::updateSelection( pfvPicker* p, int ev, pfvPickerRequest* r )
{
	if((ev==LEFT_DOWN) || (ev==RIGHT_DOWN) )
	{
		pfvInteractor* ia = p->pick();

		if( ia == (pfvInteractor*)this )
		{
			r->state = PFPICKER_MANIP;
			r->selector = this;
		}

		else if( ia == NULL )
			r->state = PFPICKER_FOCUS_EVENT;
	}
	return 1;

}
	
///////////////////////////////////////////////////////////////////////////////

void mySelector::endSelection( pfvPicker* p )
{
	int i, n = listSlaves->getNum();
	for(i=0; i<n; i++)
	{
		pfvSelector* s = (pfvSelector*)(listSlaves->get(i));
		if(s)
			s->endSelection(p);
	}
	listSlaves->reset();

	if(node)
		pfuTravNodeHlight( node, NULL );
}

///////////////////////////////////////////////////////////////////////////////


