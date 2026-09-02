
/*
 * Copyright 1995, Silicon Graphics, Inc.
 * ALL RIGHTS RESERVED
 *
 * This source code ("Source Code") was originally derived from a
 * code base owned by Silicon Graphics, Inc. ("SGI")
 * 
 * LICENSE: SGI grants the user ("Licensee") permission to reproduce,
 * distribute, and create derivative works from this Source Code,
 * provided that: (1) the user reproduces this entire notice within
 * both source and binary format redistributions and any accompanying
 * materials such as documentation in printed or electronic format;
 * (2) the Source Code is not to be used, or ported or modified for
 * use, except in conjunction with OpenGL Performer; and (3) the
 * names of Silicon Graphics, Inc.  and SGI may not be used in any
 * advertising or publicity relating to the Source Code without the
 * prior written permission of SGI.  No further license or permission
 * may be inferred or deemed or construed to exist with regard to the
 * Source Code or the code base of which it forms a part. All rights
 * not expressly granted are reserved.
 * 
 * This Source Code is provided to Licensee AS IS, without any
 * warranty of any kind, either express, implied, or statutory,
 * including, but not limited to, any warranty that the Source Code
 * will conform to specifications, any implied warranties of
 * merchantability, fitness for a particular purpose, and freedom
 * from infringement, and any warranty that the documentation will
 * conform to the program, or any warranty that the Source Code will
 * be error free.
 * 
 * IN NO EVENT WILL SGI BE LIABLE FOR ANY DAMAGES, INCLUDING, BUT NOT
 * LIMITED TO DIRECT, INDIRECT, SPECIAL OR CONSEQUENTIAL DAMAGES,
 * ARISING OUT OF, RESULTING FROM, OR IN ANY WAY CONNECTED WITH THE
 * SOURCE CODE, WHETHER OR NOT BASED UPON WARRANTY, CONTRACT, TORT OR
 * OTHERWISE, WHETHER OR NOT INJURY WAS SUSTAINED BY PERSONS OR
 * PROPERTY OR OTHERWISE, AND WHETHER OR NOT LOSS WAS SUSTAINED FROM,
 * OR AROSE OUT OF USE OR RESULTS FROM USE OF, OR LACK OF ABILITY TO
 * USE, THE SOURCE CODE.
 * 
 * Contact information:  Silicon Graphics, Inc., 
 * 1600 Amphitheatre Pkwy, Mountain View, CA  94043, 
 * or:  http://www.sgi.com
 */
///////////////////////////////////////////////////////////////////////////////

#include <Performer/pf.h>
#include <Performer/pr.h>
#include <Performer/pfutil.h>

#include <Performer/pf/pfChannel.h>
#include <Performer/pf/pfDCS.h>
#include <Performer/pf/pfGeode.h>

#include <Performer/pr/pfGeoSet.h>
#include <Performer/pr/pfGeoState.h>
#include <Performer/pr/pfLinMath.h>
#include <Performer/pr/pfMemory.h>

#include <Performer/pfv/pfvXml.h>

#include <Performer/pfv/pfvPicker.h>
#include <Performer/pfv/pfvMousePicker.h>

#include "mySlider.h"

///////////////////////////////////////////////////////////////////////////////

pfType* sliderInteractor::classType = NULL;

///////////////////////////////////////////////////////////////////////////////

sliderInteractor::sliderInteractor(pfvXmlNode* xml)
{
	if(classType==NULL)
		sliderInteractor::init();
	setType(classType);
}

sliderInteractor::~sliderInteractor()
{
    // XXX Paolo add code here
}

///////////////////////////////////////////////////////////////////////////////

void sliderInteractor::init()
{
	if(classType==NULL)
	{
		pfvInteractor::init();
		classType= new pfType(pfvInteractor::getClassType(),"sliderInteractor");
	}
}

///////////////////////////////////////////////////////////////////////////////

int sliderInteractor::specialFocus( pfvPicker* p,int ev,int prmsn,
				pfvPickerRequest*r )
{
    //printf("sliderInteractor::specialFocus called (event=%d)\n", ev );
    
    if(ev==LEFT_DOWN)
    {
	int btn = ((pfvMousePicker*)p)->getMouse()->flags & PFUDEV_MOUSE_DOWN_MASK;
	if(btn==PFUDEV_MOUSE_LEFT_DOWN)
	{	
	    r->state |= (PFPICKER_INTERACT|PFPICKER_GRAB_EVENT);
	    r->interactor = this;
	}
    }
    return 1;
}

///////////////////////////////////////////////////////////////////////////////

int sliderInteractor::startInteraction( pfvPicker*p, int ev )
{
    //printf("sliderInteractor::startInteraction - ev=%d\n", ev );
    return 1;    
}

///////////////////////////////////////////////////////////////////////////////

int sliderInteractor::updateInteraction( pfvPicker* p,int ev,int prmsn, 
pfvPickerRequest*r )

{
    //printf("sliderInteractor::updateInteraction - ev=%d\n", ev );
    if(ev==LEFT_UP)
    {
		r->state &= ~PFPICKER_INTERACT;
		r->state |= PFPICKER_FOCUS_EVENT;
		r->interactor = NULL;
		//printf("TERMINATING INTERACTION\n");
		return 1;
    }

	pfuMouse* m = ((pfvMousePicker*)p)->getMouse();

	/*pfChannel* chan = ((pfvMousePicker*)p)->chan;*/
	pfChannel* chan = p->getChannel();
	
	float x, y;
	pfuCalcNormalizedChanXY( &x, &y, chan, m->xpos, m->ypos );
	//printf("mouse x=%d y=%d  normX=%f normY=%f\n", m->xpos, m->ypos, x, y );

	float v = (x-slider->pos[0])/slider->width;
	
	if( slider->callback!=NULL )
		v = slider->callback(v,slider->cbData);
		
	slider->setValue(v);


#if 0	

    /* look at hit point */
    
    if( p->pick() == (pfvInteractor*)this )
    {    
	pfHit ** hits = p->getPickList();
	
	struct 
	{
	    int flags;
	    pfVec3 point;
	} hitbits;
	
	uint query[3] = { PFQHIT_FLAGS, PFQHIT_POINT, 0 };
	
	hits[0]->mQuery(query, &hitbits);
    
    
	if (hitbits.flags & PFHIT_POINT) 
	{
	    printf("sliderInteractor::updateInteraction - hit point: %f %f %f\n", 	
		hitbits.point[0], hitbits.point[1], hitbits.point[2] );
		
	    if(slider)
		slider->setValue( (hitbits.point[0]-slider->pos[0])/slider->width );
	    else
		printf("sliderInteractor::updateInteraction - NULL slider ptr");
		
	}
	else 
	{
	    printf("sliderInteractor::updateInteraction - no hit point for pick");
	}
    }

#endif
	
    return 1;
}

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

mySlider::mySlider(pfvPicker* p, pfvXmlNode* xmlRoot)
{
	
	//pos[0] = 0.05f;
	//pos[1] = 0.25f;
	//pos[2] = 0.9f;
	//pos[3] = 0.95f;

	pos[0] = 0.0f;
	pos[1] = 1.0f;
	pos[2] = 0.0f;
	pos[3] = 1.0f;

	value = 0.5f;

	dcs = new pfDCS;
	PFASSERTALWAYS(dcs);

	geode = new pfGeode;
	PFASSERTALWAYS(geode);
	dcs->addChild(geode);

	gset = new pfGeoSet;
	PFASSERTALWAYS(gset);
	geode->addGSet(gset);
	
	coords = (pfVec3*)pfCalloc(8,sizeof(pfVec3),pfGetSharedArena());
	PFASSERTALWAYS(coords!=NULL);

	cols = (pfVec4*)pfCalloc(2,sizeof(pfVec4),pfGetSharedArena());
	PFASSERTALWAYS(cols!=NULL);

	cols[0].set(0.9f,0.1f,0.0f,0.6f);
	cols[1].set(0.9f,0.6f,0.6f,0.5f);

	if(xmlRoot!=NULL)
	{
		int i, n = xmlRoot->getNumChildren();
	    for(i=0;i<n;i++)
	    {
			pfvXmlNode* xml = xmlRoot->getChild(i);
			printf("mySlider::mySlider - node %d is <%s>\n", 
					i, xml->getName());

			if( !xml->nameCmp("pos"))
			{
				xml->getFloatArray(4,pos);
				printf("slider pos: %f %f %f %f\n", 
						pos[0], pos[1], pos[2], pos[3] );
			}
			else if( !xml->nameCmp("value"))
			{
				xml->getFloat(&value);
				printf("slider value: %f\n", value ); 
			}
			else if( !xml->nameCmp("onColor"))
			{
				xml->getFloatArray(4,(float*)(&(cols[0])));
				printf("slider on color: %f %f %f %f\n", 
					cols[0][0], cols[0][1], cols[0][2], cols[0][3] ); 
			}
			else if( !xml->nameCmp("offColor"))
			{
				xml->getFloatArray(4,(float*)(&(cols[1])));
				printf("slider off color: %f %f %f %f\n", 
					cols[1][0], cols[1][1], cols[1][2], cols[1][3] ); 
			}
				
			else
			{
				printf("mySlider::mySlider -invalid xml token <%s>\n", 
						xml->getName());
			}

		}
	}

	width = pos[1]-pos[0];
	height = pos[3]-pos[2];
	
	coords[0].set(pos[0],0.0f,pos[3]);
	coords[1].set(pos[0],0.0f,pos[2]);

	float x = pos[0]+(value*width);

	coords[2].set(x,0.0f,pos[2]);
	coords[3].set(x,0.0f,pos[3]);
		
	coords[4] = coords[3];
	coords[5] = coords[2];
	
	coords[6].set(pos[1],0.0f,pos[2]);
	coords[7].set(pos[1],0.0f,pos[3]);
	
	
	gset->setAttr(PFGS_COORD3, PFGS_PER_VERTEX, coords, NULL);
    gset->setAttr(PFGS_COLOR4, PFGS_PER_PRIM, cols, NULL);
    gset->setPrimType(PFGS_QUADS);
    gset->setNumPrims(2);

	gstate = new pfGeoState;
	PFASSERTALWAYS(gstate);
	gstate->makeBasic();
	gstate->setMode(PFSTATE_TRANSPARENCY, PFTR_ON);
    gset->setGState(gstate);

	// interactor!
	ia = new sliderInteractor(NULL);
	PFASSERTALWAYS(ia);
	ia->slider = this;

	if(p==NULL)
	{	
		printf("mySlider::mySlider - NULL picker ptr\n");
		return;
	}
	
	picker = p;
	ia->nodeSetup( geode, picker );
}	


///////////////////////////////////////////////////////////////////////////////

mySlider::~mySlider()
{
	// XXX will everything else be deleted by deleting root??
	pfDelete(dcs);
}

///////////////////////////////////////////////////////////////////////////////

float mySlider::setValue( float v )
{
    if(v<0.0f) 
	v=0.0f;
    else if(v>1.0f) 
	v=1.0f;

    if(value==v)
	return v;
        
    value = v;
    
    float x = pos[0]+(value*width);

    coords[2].set(x,0.0f,pos[2]);
    coords[3].set(x,0.0f,pos[3]);
	    
    coords[4] = coords[3];
    coords[5] = coords[2];
    
    
}
///////////////////////////////////////////////////////////////////////////////

void mySlider::update( pfChannel* chan )
{
	if(chan==NULL)
		return;

	float _near_,_far_;
	chan->getNearFar(&_near_,&_far_);
	
	float fovX, fovY;
	chan->getFOV( &fovX, &fovY );
	
	float chanW = 2.0f*_near_*pfTan(fovX/2.0f);
	float chanH = 2.0f*_near_*pfTan(fovY/2.0f);
		
	pfMatrix chanMat,m2,m3;
	chan->getViewMat(chanMat);
	
	m2.makeTrans(-0.5,0.0f,-0.5f);

	m3.postScale(m2,chanW,1.0f,chanH);
	
	m2.postTrans(m3,0.0f,_near_*1.001,0.0f);

	m2.postMult(chanMat);
		
	dcs->setMat(m2);
}

///////////////////////////////////////////////////////////////////////////////



