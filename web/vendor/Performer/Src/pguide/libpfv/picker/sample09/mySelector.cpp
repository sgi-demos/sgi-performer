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
#include <Performer/pr/pfMemory.h>
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
	//printf("mySelector::specialFocus called (event=%d)\n", ev );
	
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
		manipEv = ev;
	}
		
	return (node!=NULL);
}

///////////////////////////////////////////////////////////////////////////////

void PreScaleDCS( pfDCS* dcs, float scale )
{
	pfMatrix m;
	dcs->getMat(m);
	pfMatrix scalemat;
	scalemat.makeScale(scale,scale,scale);
	m.preMult(scalemat);
	dcs->setMat(m);
}	

///////////////////////////////////////////////////////////////////////////////

void PreHeadDCS( pfDCS* dcs, float h )
{
	pfMatrix m;
	dcs->getMat(m);
	m.preRot(h,0.0f,0.0f,1.0f,m);
	dcs->setMat(m);
}

///////////////////////////////////////////////////////////////////////////////

void PostTransDCS( pfDCS*dcs, pfVec3 &vec )
{
	pfMatrix m;
	dcs->getMat(m);
	m.postTrans(m,vec[0],vec[1],vec[2]);
	dcs->setMat(m);
}

///////////////////////////////////////////////////////////////////////////////

int mySelector::updateManip( pfvPicker* p,int ev, pfvPickerRequest*r )
{
	if( ((manipEv==LEFT_DOWN)&&(ev==LEFT_UP)) || (ev==RIGHT_UP) )
	{
		r->state = PFPICKER_SELECT | PFPICKER_FOCUS_EVENT;
		r->selector = this;
		pfuTravNodeHlight( node, hl_sel );
		return 1;
	}
	
	pfuMouse* mouse = ((pfvMousePicker*)p)->getMouse();
	int ctrl = mouse->modifiers & 
		(PFUDEV_MOD_CTRL|PFUDEV_MOD_LEFT_CTRL|PFUDEV_MOD_RIGHT_CTRL);
	

	if( ctrl == PFUDEV_MOD_RIGHT_CTRL_SET )
	{
		float scale = (manipEv==LEFT_DOWN)?1.01f:(1.0f/1.01f);
		PreScaleDCS((pfDCS*)node,scale);
		int i, n=listSlaves->getNum();
		for(i=0;i<n;i++)
		{
			mySelector* s = (mySelector*)(listSlaves->get(i));
			PreScaleDCS((pfDCS*)(s->node),scale);
		}
	}
	
	else if( ctrl == PFUDEV_MOD_LEFT_CTRL_SET )
	{
		PreHeadDCS( (pfDCS*)node, (manipEv==LEFT_DOWN)? 1.0f : -1.0f );	

		int i, n=listSlaves->getNum();
		for(i=0;i<n;i++)
		{
			mySelector* s = (mySelector*)(listSlaves->get(i));
			PreHeadDCS((pfDCS*)(s->node),(manipEv==LEFT_DOWN)? 1.0f : -1.0f );
		}
	}
	else
	{
		pfMatrix m;
		pfDCS* dcs = (pfDCS*)node;
		dcs->getMat(m);
		pfVec3 vec,xvec;
		vec.set(0.0f,(manipEv==LEFT_DOWN)?-0.1f:0.1f,0.0);

		//printf("BEFORE XFORM: vec: %f %f %f\n", vec[0], vec[1], vec[2] );
		xvec.xformVec(vec,m);
		//printf("AFTER XFORM: xvec: %f %f %f\n", xvec[0], xvec[1], xvec[2] );
	
		PostTransDCS( dcs, xvec );

		int i, n=listSlaves->getNum();
		for(i=0;i<n;i++)
		{
			mySelector* s = (mySelector*)(listSlaves->get(i));
			PostTransDCS((pfDCS*)(s->node),xvec );
		}
	}

	return 1;
}

///////////////////////////////////////////////////////////////////////////////

int mySelector::startSelection( pfvPicker*p, int ev )
{
	if(node)
		pfuTravNodeHlight( node, hl_sel );
	
	return (node!=NULL);
}

///////////////////////////////////////////////////////////////////////////////

int mySelector::joinSelection( pfvPicker* p, pfvSelector* _master, int ev )
{
	if(!(state&(PFPICKER_SELECT|PFPICKER_MANIP)))
	{
		master = _master;
		state |= PFPICKER_SELECT;
		startSelection( p, ev );
		return 1;
	}
	printf("mySelector::joinSelection - %x is already selected?\n", this);
	return 0;
}

///////////////////////////////////////////////////////////////////////////////

int mySelector::manageSelection( pfvPicker*p, pfList*_listSlaves )
{
	if(!(state&PFPICKER_SELECT))
	{
		printf("mySelector::manageSelection - %x is not selected\n", this);
		return 0;
	}

	if( master==NULL )
	{
		printf("mySelector::manageSelection - %x's master is NULL\n", this);
		return 0;
	}

	if( _listSlaves->search(this)==-1)
	{
		printf("mySelector::manageSelection - %x not in listSlaves\n", this);
		return 0;
	}

	if( listSlaves->getNum() > 0)
	{
		printf("mySelector::manageSelection - %x already has slaves!\n", this);
		return 0;
	}

	int i, n = _listSlaves->getNum();
	for(i=0;i<n;i++)
	{
		pfvSelector* s = (pfvSelector*)(_listSlaves->get(i));
		if(s!=(pfvSelector*)this)
		{
			listSlaves->add(s);
			s->master = this;
		}
	}
	return 1;
}

///////////////////////////////////////////////////////////////////////////////


int mySelector::sanityCheck()
{
	pfvPicker::printState( "mySelector::sanityCheck - state: ", state );
	if( (!(state&PFPICKER_SELECT))
		||(state&PFPICKER_MANIP) )
	{
		printf("INVALID STATE!\n");
		return 0;
	}

	if(master!=NULL)
	{
		printf("this (%x) has a master (%x)\n", this, master);
		if( listSlaves->getNum() > 0 )
		{
			printf("listSlaves->getNum() == %d\n",listSlaves->getNum());
			return 0;
		}
	}
	else
	{
		int i,n=listSlaves->getNum();
		printf("this (%x) is the master (nSlaves=%d)\n", this, n );
		
		for(i=0;i<n;i++)
		{
			pfvSelector* s = (pfvSelector*)(listSlaves->get(i));

			printf("slave %d is: %x\n", i, s );
			if(s==NULL)	
			{
				printf("slave %d is NULL\n", i );
				return 0;
			}
			if( !(((pfvInteractor*)s)->state & PFPICKER_SELECT))
			{
				printf("slave %d (%x) is not selected\n", i, s );
				return 0;
			}
			if( s->master != this )
			{
				printf("wrong master (%x) found on slave %d (%x) \n", 
					s->master,	i, s );
				return 0;
			}
			if( s->listSlaves->getNum() > 0 )
			{
				printf("slave %d (%x) already has slaved\n", i, s );
				return 0;
			}
		}
	}
	printf("mySelector::sanityCheck - returning OK\n");
	return 1;
}
				
///////////////////////////////////////////////////////////////////////////////

int mySelector::updateSelection( pfvPicker* p, int ev, pfvPickerRequest* r )
{
	//PFASSERTALWAYS(sanityCheck());
	
	if((ev==LEFT_DOWN) || (ev==RIGHT_DOWN) )
	{
		pfuMouse* mouse = ((pfvMousePicker*)p)->getMouse();
		pfvInteractor* ia = p->pick();

		if( !(mouse->modifiers & PFUDEV_MOD_SHIFT) )
		{

			//pfvPicker::printState("\nLEFT-CLICK. req->state ", r->state );
			//printf("r->sel = %x r->int = %x\n", r->selector, r->interactor );	

			
			if( (ia==(pfvInteractor*)this) || (listSlaves->search(ia)>=0) )
			{
				r->state = PFPICKER_MANIP;
				r->selector = this;
			}
			else if(ia!=NULL)
			{	
				if(  !(r->state & PFPICKER_GRAB_EVENT) &&
						(r->state&PFPICKER_SELECT)&&(r->selector==this) ) 		
				{
					r->state = PFPICKER_FOCUS_EVENT;
				}
			}
			else
				r->state = PFPICKER_FOCUS_EVENT;

					
			return 1;
		}
		else // if shift is down
		{
			if(ia==(pfvInteractor*)this)
			{
				if( listSlaves->getNum() > 0 )
				{
					printf("should hand selection to my first slave..\n");

					pfvSelector* slave = (pfvSelector*)(listSlaves->get(0));
					if( slave->manageSelection( p, listSlaves ) )
					{
						listSlaves->reset();
						r->state = PFPICKER_SELECT|PFPICKER_FOCUS_EVENT;
						r->selector = slave;
						return 1;
					}
					else
					{
						printf("slave refused to manage selection\n");
						r->state = PFPICKER_FOCUS_EVENT;
					}
				}
				else
				{
					r->state = PFPICKER_FOCUS_EVENT;
				}
				return 1;
			}
			
			if(ia==NULL)
			{
				return 1;
			}			
			int slaveIndex=listSlaves->search(ia);
			if( slaveIndex >= 0 )
			{
				((pfvSelector*)ia)->endSelection(p);
				ia->state &= ~(PFPICKER_SELECT|PFPICKER_MANIP);
				((pfvSelector*)ia)->master = NULL;
				
				listSlaves->fastRemoveIndex(slaveIndex);
				r->state = PFPICKER_SELECT | PFPICKER_FOCUS_EVENT;
				r->selector = this;
				return 1;
			}

			if( ((pfvSelector*)ia)->joinSelection(p,this,ev) )
			{
				printf("ANOTHER SELECTOR JOINED\n");
				ia->state |= PFPICKER_SELECT;
				((pfvSelector*)ia)->master = this; 
				
				listSlaves->add(ia);
				r->state = PFPICKER_SELECT|PFPICKER_FOCUS_EVENT;
				r->selector = this;
				return 1;
			}
			printf("SELECTOR %x REFUSED TO JOIN\n", ia);
		}
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
		{
			s->endSelection(p);
			s->state &= ~(PFPICKER_SELECT|PFPICKER_MANIP);
			s->master = NULL;
		}
	}
	listSlaves->reset();

	if(node)
		pfuTravNodeHlight( node, NULL );
}

///////////////////////////////////////////////////////////////////////////////


