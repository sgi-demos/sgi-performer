
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

#include <Performer/pf/pfScene.h>
#include <Performer/pf/pfChannel.h>

#include <Performer/pr/pfList.h>

#include <Performer/pfv/pfvXml.h>

#include <Performer/pfv/pfvDisplay.h>
#include <Performer/pfv/pfvInput.h>

#include <Performer/pfv/pfvInputMngrPicker.h>
#include <Performer/pfv/pfvPicker.h>

#include "pfvmPicker.h"
#include "pfvmSelector.h"

///////////////////////////////////////////////////////////////////////////////

pfType*pfvmPicker::classType = NULL;

///////////////////////////////////////////////////////////////////////////////

void pfvmPicker::init()
{
	if(classType==NULL)
	{
		pfvModule::init();
		classType= new pfType(pfvModule::getClassType(), "pfvmPicker" );
		PFASSERTALWAYS(classType);
	}
}

///////////////////////////////////////////////////////////////////////////////

pfvmPicker::pfvmPicker(int viewIndex)
{
	if(classType==NULL)
        init();
	setType(classType);

	bindCallback(PFV_CB_POSTCONFIG);
	bindCallback(PFV_CB_FRAME);
	viewind=viewIndex;
	picker=NULL;
	first=1;
}

///////////////////////////////////////////////////////////////////////////////

pfvmPicker::~pfvmPicker()
{
	if(picker)
		pfDelete(picker);
}

///////////////////////////////////////////////////////////////////////////////

void pfvmPicker::postConfig()
{
	int i;
	int n=pfvViewer::getViewer()->getNumViews();
	
	for(i=0;i<n;i++)
	{
		if( (getViewMask()) & ((uint64_t)1<<i) )
		{
			viewind=i;
			break;
		}
	}
	
	/*printf("pfvmPicker::postConfig - viewind=%d viewmask=%d\n",
			viewind, getViewMask() );*/
	
	picker = new pfvInputMngrPicker;
	PFASSERTALWAYS(picker);
	picker->setViewIndex(viewind);

	selector = new pfvmSelector();
	PFASSERTALWAYS(selector);
}

///////////////////////////////////////////////////////////////////////////////

void pfvmPicker::frame()
{
	//printf("pfvmPicker::frame called\n");
	
	if(first)
	{
		first=0;
				
		pfvView* view = pfvViewer::getViewer()->getView(viewind);
		pfScene* scene = view->getScene();

		selector->scene = scene;

		/*printf("pfvmPicker::frame (this=%x) - inserting groups "
				"under pfScene %x\n", this, scene );*/

		pfGroup* selector_root = new pfGroup;
		pfGroup* selParent = new pfGroup;
		selParent->addChild(selector_root);
		
		int i, n = scene->getNumChildren();
		for(i=n-1;i>=0;i--)
		{
			pfNode* node = scene->getChild(i);
			scene->removeChild(node);
			selector_root->addChild(node);
		}
		scene->addChild(selParent);

		selector->root = selector_root;
		selector->nodeSetup( selParent, picker );

		pfvPickerRequest req;
		req.state = PFPICKER_SELECT | PFPICKER_FOCUS_EVENT;
		req.selector = selector;
		req.interactor = NULL;
		
		picker->grantRequest(&req);

		((pfvPicker*)picker)->setDefaultSelector(((pfvSelector*)selector));
		
	}
	else
	{
		picker->update();
	}
}

///////////////////////////////////////////////////////////////////////////////

extern "C" PFVM_DLLEXPORT
pfvModule* pfvLoadModule_pfvmPicker(pfvXmlNode*xml)
{
    pfNotify(PFNFY_INFO,PFNFY_PRINT,
	"pfvLoadModule_pfvmPicker from %s called!\n",__FILE__);
    pfvmPicker*m= new pfvmPicker();
    return (pfvModule*)m;
}	

///////////////////////////////////////////////////////////////////////////////


