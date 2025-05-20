
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
#ifndef __PFV_SELECTOR_H__
#define __PFV_SELECTOR_H__

///////////////////////////////////////////////////////////////////////////////

#include <Performer/pf/pfNode.h>
#include <Performer/pr/pfHighlight.h>

#include <Performer/pfv/pfvPicker.h>

#include "pfvmDragger.h"
#include "pfvmUndoManager.h"

///////////////////////////////////////////////////////////////////////////////

class pfvXmlNode;
class pfNode;

///////////////////////////////////////////////////////////////////////////////

class pfvmSelector : public pfvSelector
{
public:
	pfvmSelector( pfvXmlNode* xml=NULL );
	~pfvmSelector();

	static void init();
	static pfType* getClassType(){ return classType; }
	virtual const char* getTypeName() const {return "pfvmSelector";}

	int specialFocus( pfvPicker* p,int ev,int prmsn,pfvPickerRequest*r );
	
	int startSelection( pfvPicker* p, int event );
	int updateSelection( pfvPicker* p, int event, pfvPickerRequest* req );
	void endSelection( pfvPicker* p );


	void select(pfNode*_node, int addToUndoList=1);
	pfNode* getSelection() { return node; }

	static void setClipboard(pfNode*_node) { clipboard=_node; }
	static pfNode* getClipboard() { return clipboard; }

	void resizeGeom(pfNode* _node);
	

	pfNode* FIND_PREVIOUS_SIBLING( pfNode* _node );
	pfNode* FIND_NEXT_SIBLING( pfNode* _node );
	pfGroup* FIND_PARENT( pfNode* child, int* childIndex=NULL );
	pfNode* FIND_FIRST_CHILD( pfNode* _node, int childIndex=0 );
	pfNode* FIND_RECURSE_CHILD(pfGroup* parent, int childIndex=0);

	static pfType *classType;
	pfHighlight *hl_sel;
	
	pfGroup* root;

	pfScene* scene;
	pfNode* node;
	pfDCS* dcs;

	pfNode* geom;
	pfDCS* dcsGeom;

	pfvmDragger* dragger;

	pfvmUndoManager* undoMngr;


	static pfNode* clipboard;	
};

///////////////////////////////////////////////////////////////////////////////

#endif // end of __PFV_SELECTOR_H__






