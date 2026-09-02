//
// Copyright 2001, 2002 Silicon Graphics, Inc.
// ALL RIGHTS RESERVED
//
// UNPUBLISHED -- Rights reserved under the copyright laws of the United
// States.   Use of a copyright notice is precautionary only and does not
// imply publication or disclosure.
//
// U.S. GOVERNMENT RESTRICTED RIGHTS LEGEND:
// Use, duplication or disclosure by the Government is subject to restrictions
// as set forth in FAR 52.227.19(c)(2) or subparagraph (c)(1)(ii) of the Rights
// in Technical Data and Computer Software clause at DFARS 252.227-7013 and/or
// in similar or successor clauses in the FAR, or the DOD or NASA FAR
// Supplement.  Contractor/manufacturer is Silicon Graphics, Inc.,
// 2011 N. Shoreline Blvd. Mountain View, CA 94039-7311.
//
// THE CONTENT OF THIS WORK CONTAINS CONFIDENTIAL AND PROPRIETARY
// INFORMATION OF SILICON GRAPHICS, INC. ANY DUPLICATION, MODIFICATION,
// DISTRIBUTION, OR DISCLOSURE IN ANY FORM, IN WHOLE, OR IN PART, IS STRICTLY
// PROHIBITED WITHOUT THE PRIOR EXPRESS WRITTEN PERMISSION OF SILICON
// GRAPHICS, INC.
//
// pfvPicker.h
//
// $Revision: 1.4 $
// $Date: 2002/11/08 18:33:12 $
//

#ifndef __PFV_PICKER_H__
#define __PFV_PICKER_H__

///////////////////////////////////////////////////////////////////////////////

#include <stdlib.h>
#include <stdio.h>

#include <Performer/pf.h>
#include <Performer/pr.h>
#include <Performer/pr/pfGeoSet.h>
#include <Performer/pf/pfChannel.h>

#include <Performer/pfv/pfvDLL.h>

///////////////////////////////////////////////////////////////////////////////

class pfvXmlNode;
class pfvInteractor;
class pfvSelector;
class pfvPicker;
class pfScene;

///////////////////////////////////////////////////////////////////////////////

#define PFPICKER_NONE			0

#define PFPICKER_HLITE			1
#define PFPICKER_INTERACT		2
#define PFPICKER_SELECT			4
#define PFPICKER_MANIP			8

#define PFPICKER_ALLOW_HLITE		16
#define PFPICKER_FOCUS_EVENT		32
#define PFPICKER_GRAB_EVENT		64

#define PFPICKER_FULL_PERMISSIONS \
	(PFPICKER_HLITE | PFPICKER_INTERACT | PFPICKER_SELECT ) 


///////////////////////////////////////////////////////////////////////////////

typedef struct pfvPickerRequest
{
    int state;
    pfvInteractor* interactor;
    pfvSelector* selector;
	
} pfvPickerRequest;


///////////////////////////////////////////////////////////////////////////////

class PFV_DLLEXPORT pfvPicker : public pfObject
{

public:
    
    pfvPicker(pfvXmlNode* xml=NULL);
    virtual ~pfvPicker();

    static void init();
    static pfType* getClassType(){ return classType; }
    virtual const char* getTypeName() const {return "pfvPicker";}
	
    int getState() { return state; }

    static void printState(char*t, int s);

    pfScene* getScene() { return scene; }
    void setScene( pfScene* _scene ) { scene = _scene; }

    pfChannel* getChannel() { return chan; }
    void setChannel(pfChannel*_chan) { 
	chan=_chan; 
	scene=chan?chan->getScene():NULL;
    }

	    
    int setNodeDataSlot( int slot_index );
    int setNodeDataSlot( char* slot_name );
    int getNodeDataSlot() { return slotIndex; }

    virtual void update(); 

    pfvInteractor* pick();
    virtual int computeIsectSeg();
    
    void getPickResults(  pfvInteractor** ia ) {if(ia)*ia=p_ia; } 
    void getPickResults(  pfNode** node ) {if(node)*node=p_node; } 
    pfHit ** getPickList() { return picklist; }
    
    pfvSelector* getDefaultSelector(){
	    return(defaultSelector==nullSelector)?NULL:defaultSelector;}
    pfvSelector* setDefaultSelector(pfvSelector* s){defaultSelector = s; return s;}
    
    pfvSelector* getCurSelector(){return(selector==nullSelector)?NULL:selector;}
    pfvInteractor* getCurInteractor(){return interactor;}

    virtual int grantRequest(pfvPickerRequest* req, int event=0);

    virtual int setState(int s, pfvSelector*sel, pfvInteractor* ia){
	pfvPickerRequest req={s, ia, sel};
	return grantRequest(&req);
    }
        
    virtual int collectEvents();
    pfList* getEventList() { return listEvents; }
	 
	
protected:

    static pfType *classType;

    int state;
    int slotIndex;

    pfScene* scene;
    pfChannel*chan;

    int pickFrameCount;
    pfSeg seg;

    int pickMode;
    int isectMask;
    
    pfList *listHits;
    pfHit ** picklist;

    pfvInteractor* p_ia;
    pfNode* p_node;
	
    pfvInteractor* interactor;
	
    pfvSelector* nullSelector;
    pfvSelector* defaultSelector;
    pfvSelector* selector;
    
    pfList* listEvents;
	
private:

    void construct();
    int cleanUpRequest(pfvPickerRequest*req);
};


///////////////////////////////////////////////////////////////////////////////

class PFV_DLLEXPORT pfvInteractor : public pfObject
{

public:
    pfvInteractor(pfvXmlNode* xml=NULL);
    virtual ~pfvInteractor();

    static void init();
    static pfType* getClassType(){ return classType; }
    virtual const char* getTypeName() const {return "pfvInteractor";}

    virtual int getActive(pfvPicker*p);
    //virtual int setActive(int a);

    virtual int startHlite( pfvPicker*p, int prmsn );
    virtual int updateHlite( pfvPicker* p,int ev,int prmsn, pfvPickerRequest*r );
    virtual void endHlite( pfvPicker* p );

    virtual int specialFocus( pfvPicker* p,int ev,int prmsn,pfvPickerRequest*r );

    virtual int startInteraction( pfvPicker*p, int ev /*,int prmsn*/ );
    virtual int updateInteraction( pfvPicker* p,int ev,int prmsn, pfvPickerRequest*r );
    virtual void endInteraction( pfvPicker* p );

    
    int getState() { return state; }
    int isHlited() { return ((state&PFPICKER_HLITE)!=0); }
    int isInteracting() { return ((state&PFPICKER_INTERACT)!=0); }

        
    virtual void nodeSetup( pfNode* node, int slotIndex )
    { if(node!=NULL)((pfObject*)node)->setUserData( slotIndex, this ); }

    void nodeSetup( pfNode* node, pfvPicker* p )
    { nodeSetup(node, p->getNodeDataSlot()); }

    void nodeSetup( pfNode* node, char*slotName )
    { nodeSetup(node,pfObject::getNamedUserDataSlot((const char*)slotName)); }

    void nodeSetup( pfNode*node )
	{ nodeSetup(node,"PFPICKER"); }
		
	
    static void clearNodeSlot( pfNode* node, int slotIndex )
    { if(node!=NULL)((pfObject*)node)->setUserData( slotIndex, NULL ); }

    static void clearNodeSlot( pfNode* node, pfvPicker* p )
    { clearNodeSlot(node, p->getNodeDataSlot()); }

    static void clearNodeSlot( pfNode* node, char*slotName )
    { clearNodeSlot(node,pfObject::getNamedUserDataSlot((const char*)slotName)); }

    static void clearNodeSlot( pfNode*node ) { clearNodeSlot(node,"PFPICKER"); }
	
	

// XXX making this public for now.. must be protected!
/// protected:
public:
	
	static pfType* classType;

friend class pfvPicker;

	int state;
	
}; 

///////////////////////////////////////////////////////////////////////////////


class PFV_DLLEXPORT pfvSelector : public pfvInteractor
{

public:

    pfvSelector(pfvXmlNode*xml=NULL);
    virtual ~pfvSelector();


    static void init();
    static pfType* getClassType(){ return classType; }
    virtual const char* getTypeName() const {return "pfvSelector";}

    
    virtual int startSelection( pfvPicker* p, int event );
		    
    virtual int updateSelection( pfvPicker* p, int event, pfvPickerRequest* req );

    virtual void endSelection( pfvPicker* p );

    virtual int toggleSelection( pfvPicker* p, pfvSelector* slave ); 
    virtual int joinSelection( pfvPicker* p, pfvSelector* master, int ev );
    virtual int manageSelection( pfvPicker* p, pfList* slaves );


    virtual int startManip( pfvPicker* p, int event );

    virtual int updateManip( pfvPicker* p, int event, pfvPickerRequest* req );    
    
    pfvSelector* getMaster() { return master; }
    pfList* getSlaves() { return ((listSlaves->getNum()>0)? listSlaves:NULL);} 

    virtual int getPermissions( pfvPicker* p, pfvInteractor* ia, int event, int reason );

    int isSelecting() { return ((getState()&(PFPICKER_SELECT|PFPICKER_MANIP))!=0); }
    int isManipulating() { return ((getState()&PFPICKER_MANIP)!=0); }


// XXX making this public for now.. must become protected!
// 	
///protected:
public:

    static pfType *classType;

friend class pfvPicker;

    pfvSelector* master;
    pfList* listSlaves;

};


///////////////////////////////////////////////////////////////////////////////


#endif /* end of __PFV_PICKER_H__ */
		  

