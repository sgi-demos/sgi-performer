
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

#include <Performer/pr/pfList.h>

#include "pfvmUndoManager.h"

///////////////////////////////////////////////////////////////////////////////

void CLEAR_LIST( pfList* list )
{
    if(list)
    {
	int i, n = list->getNum();
	for(i=0; i<n; i++)
	{
	    UndoableAction* action = (UndoableAction*)(list->get(i));
	    delete action;
	}
	list->reset();
    }
}

///////////////////////////////////////////////////////////////////////////////

pfvmUndoManager::pfvmUndoManager()
{
    maxUndos = 4096;
    
    listUndo = new pfList;
    listRedo = new pfList;
    
    PFASSERTALWAYS( (listUndo!=NULL) && (listRedo!=NULL) );    
}

///////////////////////////////////////////////////////////////////////////////

pfvmUndoManager::~pfvmUndoManager()
{
    CLEAR_LIST(listUndo);
    //pfDelete(listUndo);
    //delete listUndo;
    
    CLEAR_LIST(listRedo);    
    //pfDelete(listRedo);
}    
    
///////////////////////////////////////////////////////////////////////////////

int pfvmUndoManager::add( UndoableAction* action )
{
    CLEAR_LIST(listRedo);
    listUndo->add(action);
    
    while(listUndo->getNum() > maxUndos )
    {
	UndoableAction* action = (UndoableAction*)(listUndo->get(0));
	delete action;
	listUndo->removeIndex(0);
    }
    
    return listUndo->getNum();
}

///////////////////////////////////////////////////////////////////////////////

int pfvmUndoManager::undo()
{
    int n = listUndo->getNum();
    if(n>0)
    {
	UndoableAction* action = (UndoableAction*)(listUndo->get(n-1));
	if(action->undo()==0)
	{
	    /*printf("failed to undo action %x\n", action );*/
	    CLEAR_LIST(listUndo);
	    return 0;
	}
	else
	{
	    /*printf("undid action %x\n", action );*/
	    listUndo->removeIndex(n-1);
	    listRedo->add(action);
	    return 1;
	}
    }
    return 0;
}

///////////////////////////////////////////////////////////////////////////////

int pfvmUndoManager::redo()
{
    int n = listRedo->getNum();
    if(n>0)
    {
	UndoableAction* action = (UndoableAction*)(listRedo->get(n-1));
	if(action->redo()==0)
	{
	    /*printf("failed to redo action %x\n", action );*/
	    CLEAR_LIST(listRedo);
	    return 0;
	}
	else
	{
	    /*printf("redid action %x\n", action );*/
	    listRedo->removeIndex(n-1);
	    listUndo->add(action);
	    return 1;
	}
    }
    return 0;
}

///////////////////////////////////////////////////////////////////////////////

