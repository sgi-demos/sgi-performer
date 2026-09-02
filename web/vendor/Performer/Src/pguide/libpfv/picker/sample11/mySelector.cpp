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

	hl_sel = new pfHighlight;
	PFASSERTALWAYS(hl_sel);
	hl_sel->setMode( PFHL_LINES );
 	hl_sel->setColor( PFHL_FGCOLOR, 0.15f, 0.5f, 0.5f );

	listSel = new pfList;
	PFASSERTALWAYS(listSel);
}


///////////////////////////////////////////////////////////////////////////////

mySelector::~mySelector()
{
	pfDelete(hl_sel);
	pfDelete(listSel);
}

///////////////////////////////////////////////////////////////////////////////

int mySelector::select( pfNode* node )
{
	//printf("mySelector::select(%x)\n", node );

	if( listSel->search((void*)node) == -1 )
	{
		listSel->add((void*)node);
		pfuTravNodeHlight( node, hl_sel );
		return 1;
	}
	
	return 0;
}

///////////////////////////////////////////////////////////////////////////////

int mySelector::deselect( pfNode* node )
{
	//printf("mySelector::deselect(%x)\n", node );

	int i = listSel->search((void*)node);
	if( i >= 0 )
	{
		listSel->removeIndex(i);
		pfuTravNodeHlight( node, NULL );
		return 1;
	}
	
	return 0;
}

///////////////////////////////////////////////////////////////////////////////

int mySelector::deselectAll()
{
	//printf("mySelector::deselectAll()\n" );
	
	int i, n = listSel->getNum();
	for(i=0; i<n; i++)
	{
		pfNode* node = (pfNode*)(listSel->get(i));
		if(node)
			pfuTravNodeHlight( node, NULL );
	}
	listSel->reset();
	return 1;
}
	
///////////////////////////////////////////////////////////////////////////////

int mySelector::toggle( pfNode* node )
{
	//printf("mySelector::toggle(%x)\n", node );

	int i = listSel->search((void*)node);
	if( i >= 0 )
	{
		listSel->removeIndex(i);
		pfuTravNodeHlight( node, NULL );
		return 1;
	}
	else
	{
		listSel->add((void*)node);
		pfuTravNodeHlight( node, hl_sel );
		return 1;
	}
}

///////////////////////////////////////////////////////////////////////////////

int mySelector::specialFocus( pfvPicker* p,int ev,int prmsn,pfvPickerRequest*r )
{
	//printf("mySelector::specialFocus called (event=%d)\n", ev );
	
	if((ev==LEFT_DOWN))
	{
		r->state = PFPICKER_SELECT | PFPICKER_FOCUS_EVENT;
		r->selector = this;
	}
	return 1;
}

//////////////////////////////////////////////////////////////////////////////

int mySelector::startSelection( pfvPicker*p, int ev )
{
	//printf("mySelector::startSelection called\n");
	return 1;
}

///////////////////////////////////////////////////////////////////////////////

void mySelector::printSel( char* prefix )
{
	if(prefix)
		printf( prefix );
	int i, n = listSel->getNum();
	
	printf(" nSelect=%d - ", n );
	for(i=0;i<n;i++)
		printf("%x ", listSel->get(i) );
	printf("\n");
}

///////////////////////////////////////////////////////////////////////////////

int mySelector::updateSelection( pfvPicker* p, int ev, pfvPickerRequest* r )
{
	
	//printf("mySelector::updateSelection -ev=%d\n", ev );

	
	if((ev==LEFT_DOWN))
	{
		pfvInteractor* ia = p->pick();

		pfNode *node;
		p->getPickResults( &node );

		//printf("picked node=%x\n", node );

		if(node==NULL)
			deselectAll();

		else if( ia == (pfvInteractor*)this )
			toggle(node);	
	}

	//printSel();
	return 1;

}
	
///////////////////////////////////////////////////////////////////////////////

void mySelector::endSelection( pfvPicker* p )
{
	deselectAll();
}

///////////////////////////////////////////////////////////////////////////////


