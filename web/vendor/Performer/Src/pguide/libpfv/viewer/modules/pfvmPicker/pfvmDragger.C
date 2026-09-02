
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

#include <Performer/pr/pfHighlight.h>

#include <Performer/pf.h>
#include <Performer/pf/pfNode.h>
#include <Performer/pf/pfDCS.h>
#include <Performer/pf/pfTraverser.h>

#include <Performer/pfutil.h>

#include <Performer/pfv/pfvInputMngrPicker.h>

#include "pfvmDragger.h"

///////////////////////////////////////////////////////////////////////////////

pfType* pfvmDragger::classType = NULL;

///////////////////////////////////////////////////////////////////////////////

void pfvmDragger::init()
{
	if(classType==NULL)
	{
		pfvInteractor::init();
		classType= new pfType(pfvInteractor::getClassType(), "pfvmDragger" );
	}
}

///////////////////////////////////////////////////////////////////////////////

pfvmDragger::pfvmDragger(pfvmUndoManager*undo, pfvXmlNode* xml)
{
	if(classType==NULL)
		init();
	setType(classType);

	node = NULL;
	transProjector = new pfvmTransProjector;
	rotProjector = new pfvmRotProjector;
		
	undoMngr = undo;
}


///////////////////////////////////////////////////////////////////////////////

pfvmDragger::~pfvmDragger()
{
	delete transProjector;
	delete rotProjector;
}

///////////////////////////////////////////////////////////////////////////////

int pfvmDragger::specialFocus( pfvPicker* p,int ev,int prmsn,pfvPickerRequest*r )
{

    /*printf("pfvmDragger::specialFocus called! event=%d\n");*/
	
    if(ev==LEFT_DOWN)
    {
        int btn = ((pfvInputMngrPicker*)p)->getMouse()->flags & PFUDEV_MOUSE_DOWN_MASK;
        if(btn==PFUDEV_MOUSE_LEFT_DOWN)
        {	
            r->state |= PFPICKER_INTERACT;
            r->interactor = (pfvInteractor*)this;
            dragAction = 2; /* rotation action */
            /*printf("pfvmDragger::specialFocus - requesting rotation action\n");*/
        }
    }
    if(ev==RIGHT_DOWN)
    {
        int btn = ((pfvInputMngrPicker*)p)->getMouse()->flags & PFUDEV_MOUSE_DOWN_MASK;
        if(btn==PFUDEV_MOUSE_RIGHT_DOWN)
        {	
            if(((pfvInputMngrPicker*)p)->getMouse()->modifiers & PFUDEV_MOD_SHIFT)
            {
                dragAction = 0; /*translation along xy plane */
                /*printf("pfvmDragger::specialFocus - requesting"
                    "translation along xy plane\n");*/
            }
            else
            {
                dragAction = 1; /* translation along channel view plane */
                /*printf("pfvmDragger::specialFocus - requesting"
                    "translation along channel view plane\n");*/
            }
            r->state |= PFPICKER_INTERACT;
            r->interactor = (pfvInteractor*)this;
        }
    }    
    return 1;        
}

///////////////////////////////////////////////////////////////////////////////

int pfvmDragger::startInteraction( pfvPicker*p, int ev )
{
	/*printf("\npfvmDragger::startInteraction (node=%x)\n\n",node);*/
	if(node)
	{
		/*printf("pfvmDragger::startInteraction - calling picker->pick()\n");*/

		p->pick();
		pfHit** daPickList = p->getPickList();
		pfPath *daPath;
		daPickList[0]->query(PFQHIT_PATH, &daPath);

		int pathlen = daPath->getNum();
		int index = daPath->search(node);

		/*printf("pfvmDragger::startInteraction - index=%d\n", index );*/
		if( index<0 )
		{
			/*printf("pfvmDragger::startInteraction - "
				"node %x is not in picked path\n", node);*/
			return 0;
		}
		
		//truncate path so that last element in path is selected dcs
		daPath->setNum(index+1);

		pfChannel* _chan = ((pfvInputMngrPicker*)p)->getChan();
		pfuMouse*m=((pfvInputMngrPicker*)p)->getMouse();
		pfuMapMouseToChan(m, _chan);

		if(dragAction==2)
		{
			rotProjector->setChan(_chan);
			rotProjector->setPath(daPath);
			rotProjector->project(m->xchan, m->ychan);
		}
		else
		{
			transProjector->setChan(_chan);
			transProjector->setPath(daPath);
			transProjector->project(m->xchan, m->ychan);
		}
		
    	//restore path so that other interactors may look at it
		daPath->setNum(pathlen);

		((pfDCS*)node)->getMat(oldMat);

	}
	return (node!=NULL);
}

///////////////////////////////////////////////////////////////////////////////

int pfvmDragger::updateInteraction( pfvPicker* p,int ev,  int prmsn, pfvPickerRequest*r )
{

	/*printf("pfvmDragger::updateInteraction\n");*/
	
	if(ev==RIGHT_UP && dragAction<2 || ev==LEFT_UP )
	{
		r->state |= PFPICKER_FOCUS_EVENT;
		r->state &= ~PFPICKER_INTERACT;
		r->interactor = NULL;
	}
	else		
	{
		pfMatrix mat;
		pfDCS*dcs = (pfDCS*)node;
		dcs->getMat(mat);

		pfChannel* _chan = ((pfvInputMngrPicker*)p)->getChan();
		pfuMouse*m=((pfvInputMngrPicker*)p)->getMouse();
		pfuMapMouseToChan(m, _chan);

		pfMatrix rotmat;
		if(dragAction==2)
			rotProjector->projectAndGetRotation(m->xchan,m->ychan,rotmat);
		else
			transProjector->projectAndGetRotation(m->xchan,m->ychan,rotmat,dragAction);
		
		dcs->setMat(rotmat);
	}
	
	return 1;
}

///////////////////////////////////////////////////////////////////////////////

void pfvmDragger::endInteraction( pfvPicker* p )
{
    pfMatrix newMat;
    ((pfDCS*)node)->getMat(newMat);
    if(newMat!=oldMat)
    {
        /*printf("ADDING DCS ACTION TO UNDO LIST\n");*/
        undoMngr->add( (UndoableAction*)
        (new UndoableDCSAction((pfDCS*)node, oldMat, newMat)) );
    }
    else
    {
        /*printf("NOT ADDING DCS ACTION TO UNDO LIST (Matrix unchanged)\n");*/
    }
}

///////////////////////////////////////////////////////////////////////////////


