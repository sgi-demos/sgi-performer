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
// pfvViewer.h
//
// $Revision: 1.13 $
// $Date: 2002/11/11 03:03:17 $
//

#ifndef __PFV_VIEWER_H__
#define __PFV_VIEWER_H__

///////////////////////////////////////////////////////////////////////////////

#include <stdlib.h>
#include <stdio.h>

#include <Performer/pf.h>
#include <Performer/pr.h>

#include <Performer/pr/pfList.h>
#include <Performer/pf/pfScene.h>

#include <Performer/pfv/pfvDisplay.h>
#include <Performer/pfv/pfvInput.h>
#include <Performer/pfv/pfvDLL.h>

///////////////////////////////////////////////////////////////////////////////

#define PFV_SCOPE_GLOBAL 	1
#define PFV_SCOPE_VIEW   	2
#define PFV_SCOPE_WORLD  	3

///////////////////////////////////////////////////////////////////////////////

#define PFV_NUM_CALLBACKS	13

#define PFV_CB_PRECONFIG 	1
#define PFV_CB_POSTCONFIG	2
#define PFV_CB_SYNC		    3
#define PFV_CB_FRAME		4
#define PFV_CB_PRECULL 		5
#define PFV_CB_POSTCULL		6
#define PFV_CB_PREDRAW 		7
#define PFV_CB_POSTDRAW		8
#define PFV_CB_OVERLAY		9
#define PFV_CB_ENTER_VIEW	10
#define PFV_CB_EXIT_VIEW	11
#define PFV_CB_ENTER_WORLD	12
#define PFV_CB_EXIT_WORLD	13


///////////////////////////////////////////////////////////////////////////////

#define PFV_STAGE_PRECONSTRUCT	    1 /* a viewer has not yet been created */
#define PFV_STAGE_POSTCONSTRUCT     2 /* viewer has been created, not configed */

#define PFV_STAGE_PRECONFIG	    3 /* viewer is dispatching PRECONFIG cbs */
#define PFV_STAGE_POSTCONFIG	    4 /* viewer is dispatching POSTCONFIG cbs */

#define PFV_STAGE_CONFIGED	    5 /* v->config() is done, v->frame not called */

#define PFV_STAGE_RUNTIME	    6 /* viewer->frame() already called */


///////////////////////////////////////////////////////////////////////////////

class pfvXmlNode;
class pfvViewer;
class pfvModule;

///////////////////////////////////////////////////////////////////////////////

extern PFV_DLLEXPORT void pfvReplaceString(char**oldStr,char*newStr);

///////////////////////////////////////////////////////////////////////////////

class PFV_DLLEXPORT pfvWorld : public pfvObject
{
private:
	friend class pfvViewer;
	friend class pfvView;

	pfvWorld();
	~pfvWorld();

	void postConfig();
	
	static void init();

public:
	static pfType* getClassType(){ return classType; }
	virtual const char* getTypeName() const {return "pfvWorld";}

	/*void parseXml( pfvXmlNode* xml );*/

	int getNumViews() { return listViews->getNum(); }
	pfvView* getView(int i=0){ return (pfvView*)(listViews->get(i)); }
	
	pfScene* getScene() { return scene; }

	int addNode(pfNode*node){
		if(node && scene) return scene->addChild(node);
		else return 0;
	}
	
	uint64_t getViewMask() { return viewMask; }
	
protected:
	static pfType *classType;

private:
	static pfvViewer* viewer;

	void construct();
	void dispatchCallbacks(int which, pfvView*v);

	pfScene* scene;

	void joinView(pfvView*v);
	void leaveView(pfvView*v);
	pfList* listViews;
	uint64_t viewMask;

	void addModule(pfvModule*m){
		if(listModules->search(m)==-1)
			listModules->add(m);
	}
	pfList* listModules;
} ;

///////////////////////////////////////////////////////////////////////////////

class PFV_DLLEXPORT pfvView : public pfvDispView
{
private:
	friend class pfvViewer;
	pfvView();
	~pfvView();
	static void init();
	
	void postConfig();

public:
	static pfType* getClassType(){ return classType; }
	virtual const char* getTypeName() const {return "pfvView";}

#if 0
	pfvWorld* getWorld() { return world; }
	int setWorld(pfvWorld*world);
	int setWorld(int index);
#endif

	virtual int setXmlField( pfvXmlNode*xml );

	
	pfvWorld* getCurWorld() { return curWorld; }

	int setTargetWorld( pfvWorld* w );
	int setTargetWorld( char* wName );
	int setTargetWorld( int wIndex );
	
	pfvWorld* getTargetWorld();
	
	char* getTargetWorldName(){
	    return targetWorld? targetWorld->getName() : targetWorldName;
	}


	void autoPos();
	void setEye( pfVec3& xyz, pfVec3& hpr );
	
	int addNode(pfNode*node){
	    if(rootGroup==NULL || node==NULL)
		return 0;
	    rootGroup->addChild(node);
	    return 1;
	}
	
	pfGroup* getRootNode() { return rootGroup; }

	
protected:
	static pfType *classType;
	static pfvViewer* viewer;

private:
	void construct();
	void switchWorld();
	void dispatchCallbacks(int which);

	void addModule(pfvModule*m){
		if(listModules->search(m)==-1)
			listModules->add(m);
	}
	

#if 0
	pfvWorld* world;
#endif

	pfvWorld* curWorld;
	pfvWorld* targetWorld;
	char* targetWorldName;
	
	pfGroup* rootGroup;
	
	pfList* listModules;

	int flagAutoPos;
} ;

///////////////////////////////////////////////////////////////////////////////

class PFV_DLLEXPORT pfvModule : public pfvObject
{
private:
	friend class pfvViewer;
	friend class pfvWorld;	
	friend class pfvView;	
public:
	pfvModule();
	virtual ~pfvModule();

	static void init();
	static pfType* getClassType(){ return classType; }
	virtual const char* getTypeName() const {return "pfvModule";}

	/*int parseXml( pfvXmlNode* xml );*/



	static pfvModule* load( char*className, 
			char* dsoName=NULL, pfvXmlNode*xml=NULL );
	
	static pfvModule* load( char*className, 
			pfvXmlNode*xml, char* dsoName=NULL ){
		return load(className,dsoName,xml);
	}

	static char* getLoadPath() { return loadPath; }
	static void setLoadPath(char*path); 	
	


	int getScope() { return scope; }
	int getScopeIndex() { return scopeIndex; }
	pfvObject* getScopeTarget() { return scopeTarget; }



	
	void bindCallback(int which);
	void unbindCallback(int which);
	
	int getCBMask() { return cbMask; }

	virtual void preConfig(){ pfNotify(PFNFY_NOTICE, PFNFY_PRINT, "pfvModule::preConfig(this=%x)\n",this); }
	virtual void postConfig(){ pfNotify(PFNFY_NOTICE, PFNFY_PRINT, "pfvModule::postConfig(this=%x)\n",this); }

	virtual void enterView(pfvView*v){ 
	    pfNotify(PFNFY_NOTICE, PFNFY_PRINT, "pfvModule::enterView(this=%x,view=%x)\n",this, v); 
	}

	virtual void exitView(pfvView*v){ 
	    pfNotify(PFNFY_NOTICE, PFNFY_PRINT, "pfvModule::exitView(this=%x,view=%x)\n",this, v); 
	}
	
	virtual void enterWorld(){ pfNotify(PFNFY_NOTICE, PFNFY_PRINT, "pfvModule::enterWorld(this=%x)\n",this); }
	virtual void exitWorld(){ pfNotify(PFNFY_NOTICE, PFNFY_PRINT, "pfvModule::exitWorld(this=%x)\n",this); }
    

	virtual void sync(){ pfNotify(PFNFY_NOTICE, PFNFY_PRINT, "pfvModule::sync(this=%x)\n",this); }
	virtual void frame(){ pfNotify(PFNFY_NOTICE, PFNFY_PRINT, "pfvModule::frame(this=%x)\n",this); }


	virtual void preCull(pfvDispChan*chan){ 
		pfNotify(PFNFY_NOTICE, PFNFY_PRINT, "pfvModule::preCull(this=%x,chan->index=%d)\n", this,chan->getIndex()); 
	}

	virtual void postCull(pfvDispChan*chan){ 
		pfNotify(PFNFY_NOTICE, PFNFY_PRINT, "pfvModule::postCull(this=%x,chan->index=%d)\n", this,chan->getIndex()); 
	}

	virtual void preDraw(pfvDispChan*chan){ 
		pfNotify(PFNFY_NOTICE, PFNFY_PRINT, "pfvModule::preDraw(this=%x,chan->index=%d)\n", this,chan->getIndex()); 
	}
	
	virtual void postDraw(pfvDispChan*chan){ 
		pfNotify(PFNFY_NOTICE, PFNFY_PRINT, "pfvModule::preDraw(this=%x,chan->index=%d)\n", this,chan->getIndex()); 
	}

	virtual void overlay(pfvDispChan*chan){ 
		pfNotify(PFNFY_NOTICE, PFNFY_PRINT, "pfvModule::overlay(this=%x,chan->index=%d)\n", this,chan->getIndex()); 
	}




	// pfvInputMngr Callbacks
	char* getEventMask() { return inputCB? inputCB->getEventMask() : (char*)""; }
	void setEventMask(char* str);

	int bindKeys(char*keys);
	int unbindKeys(char*keys) { return (inputCB)? inputCB->unbindKeys(keys) : 0; }

	virtual int handleEvent(int evType, char key) { return 0; }

	uint64_t getViewMask() { return viewMask; }

	
protected:
	static pfType *classType;
	static pfvViewer* viewer;
	
private:

	static char* loadPath;

	void construct();

	void joinViewer(int _scope, int _scopeIndex );
	
	void joinView(pfvView*v);
	void leaveView(pfvView*v);

	int scope;
	int scopeIndex;
	pfvObject* scopeTarget;

	int cbMask;
	
	pfvInputMngrCallback* inputCB;
	
	uint64_t viewMask;

};

///////////////////////////////////////////////////////////////////////////////

class PFV_DLLEXPORT pfvViewer : public pfvDisplayMngr
{
public:
	pfvViewer();
    pfvViewer(char* filename);
    pfvViewer(pfvXmlNode* xml);
    virtual ~pfvViewer();
	
	static void init();
	static pfType* getClassType(){ return classType; }
	virtual const char* getTypeName() const {return "pfvViewer";}

	int parseXml( pfvXmlNode* xml );
	
	static pfvViewer* getViewer() { return viewer; }


	void config();
	void sync();
	void frame();
	void run(float time=-1.0f);


	pfvWorld* createWorld(); 
	pfvWorld* createWorld(pfvXmlNode* xml);
	pfvWorld* createWorld(char* name);
	int getNumWorlds() { return listWorlds->getNum(); }
	
	pfvWorld* getWorld(int i=-1) { 
		if(i<0 && curView)
			return curView->getCurWorld();
		else
			return (pfvWorld*)(listWorlds->get(i)); 
	}
	
	pfvWorld* findWorld(char*_name){
		return (pfvWorld*)pfvObject::find(listWorlds,_name);
	}

	int addNode(pfNode*node){
		return (curWorld)? curWorld->addNode(node) : 0;
	}

	pfvWorld* getCurWorld() { return curWorld; }
	int getCurWorldIndex() { return curWorld? curWorld->getIndex() : -1; }
	void setCurWorld(pfvWorld*w) { curWorld = w; }
	void setCurWorldIndex(int i) { setCurWorld(getWorld(i)); }



	pfvDispView* createView(); 
	pfvDispView* createView(pfvXmlNode* xml);
	pfvDispView* createView(char* name);

	pfvView* getView(int i) { return (pfvView*)(listViews->get(i)); }
	pfvView* findView(char*_name){
    	return (pfvView*)pfvObject::find(listViews,_name); 
	}


	
	void autoPos(){
		if(curView) curView->autoPos();
	}

	void setTargetWorld(pfvWorld*w) { if(curView)curView->setTargetWorld(w); }
	void setTargetWorld(int i) { if(curView)curView->setTargetWorld(i); }

	pfvView* getCurView() { return curView; }
	int getCurViewIndex() { return curView? curView->getIndex() : -1; }
	void setCurView(pfvView*v) { curView = v; }
	void setCurViewIndex(int i) { setCurView((pfvView*)getView(i)); }

	

	int addModule( 	pfvModule*m, 
					int scope=PFV_SCOPE_GLOBAL, 
					int scopeIndex=-1 );

	int getNumModules() { return listModules->getNum(); }
	pfvModule* getModule(int i) { return (pfvModule*)(listModules->get(i)); }

	pfvModule* findModule(	char*_name ){
		return (pfvModule*)pfvObject::find(listModules,_name);
	}
	pfvModule* findModule(	char*name,
							pfType*classType,
							pfObject*scopeTarget=NULL);
	pfvModule* findModule(	char*name,
							pfObject*scopeTarget,
							pfType*classType=NULL)
	{
		return findModule(name,classType,scopeTarget);
	}

	static const char* getCallbackName(int cb)
	{
		if((cb>0)&&(cb<=PFV_NUM_CALLBACKS))
			return cbNames[cb];
		return "NULL";
	}	
	static char* getCBNames(int cbmask);
	
	int dispatchCallback(int cb, uint64_t viewMask, pfvDispChan*chan);
	
	
	static int getStage() { return stage; }	



protected:
	static pfType *classType;
	
private:

	static int stage;

	static const char* cbNames[PFV_NUM_CALLBACKS+1]; 
	static pfvViewer* viewer;

	void construct();

friend class pfvView;

	int scheduleWorldChange(pfvView*v) {	    
	    if(listWorldChanges->search(v)==-1)
	    {
		pfNotify(PFNFY_INFO, PFNFY_PRINT, 
		"adding view %d to listWorldChanges", v->getIndex() );
		listWorldChanges->add(v);
	    }
	    return 1;
	}

	int removeScheduledWorldChange(pfvView*v){
	    pfNotify(PFNFY_INFO, PFNFY_PRINT, 
	    "removing view %d from listWorldChanges", v->getIndex() );
	    listWorldChanges->remove(v);
	    return 1;
	}

	friend void pfvChannelDrawFunc(pfChannel*pfchan, void *data);
	void drawChannel( pfvDispChan* chan );
	
	int parseXmlCreateModule(pfvXmlNode*xmlRoot);
	pfList* listModules;
	
	pfList* listWorlds;
	pfvWorld* curWorld;

	pfvView* curView;

	pfList* listWorldChanges;
	int syncCalled;

};


///////////////////////////////////////////////////////////////////////////////


#endif /* end of __PFV_VIEWER_H__ */
		  

