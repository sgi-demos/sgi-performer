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
// pfvDisplay.h
//
// $Revision: 1.5 $
// $Date: 2002/11/08 18:33:12 $
//

#ifndef __PFV_DISPLAY_H__
#define __PFV_DISPLAY_H__

///////////////////////////////////////////////////////////////////////////////

#include <Performer/pr/pfList.h>
#include <Performer/pr/pfGeoMath.h>
#include <Performer/pr/pfLinMath.h>
#include <Performer/pfutil.h>
#include <Performer/pf/pfNode.h>

#include <Performer/pfv/pfvDLL.h>

///////////////////////////////////////////////////////////////////////////////

extern int pfvSetNodeChanMask( pfNode* node, uint64_t m );

///////////////////////////////////////////////////////////////////////////////

class pfvDispPipe;
class pfvDispPWin;
class pfvDispChan;
class pfvDispView;
class pfPipe;
class pfPipeWindow;
class pfChannel;
class pfScene;
class pfvXmlNode;

///////////////////////////////////////////////////////////////////////////////

class PFV_DLLEXPORT pfvObject : public pfObject
{
public:
    pfvObject();
    pfvObject(char*_name, int _index=-1);
    pfvObject(int _index);
    virtual ~pfvObject();
	
    static void init();
    static pfType* getClassType() { return classType; }
    virtual const char* getTypeName() const {return "pfvObject";}

    virtual int parseXml(pfvXmlNode*xml);
    virtual int setXmlField(pfvXmlNode*xml);
    
    char* getName(){ return name? name : (char*)"NULL_NAME" ;}
    void setName(char*_name);

    int getIndex() { return index; }

    static pfvObject* find(pfList*list, char*_name);
	
protected:
    void setIndex(int i) { index = i; }

private:
    void construct(char* n, int i);

friend class pfvDisplayMngr;

    static pfType* classType;

    int index;
    char* name;
};

///////////////////////////////////////////////////////////////////////////////

class PFV_DLLEXPORT pfvDisplayMngr : public pfObject
{

public:
    pfvDisplayMngr();
    pfvDisplayMngr(char* filename);
    pfvDisplayMngr(pfvXmlNode* xml);
    virtual ~pfvDisplayMngr();

private:
    void construct();

public:	
    static void init();
    static pfType* getClassType() { return classType; }
    virtual const char* getTypeName() const {return "pfvDisplayMngr";}
	
    static pfvDisplayMngr* getMngr() { return mngr; }
	
    virtual int loadFile(char* filename);
    virtual int parseXml(pfvXmlNode* xml);
	
    virtual void preConfig();
    virtual void postConfig();

    // pipes

    virtual pfvDispPipe* createPipe(pfvXmlNode* xml=NULL);

    int getNumPipes() { return listPipes->getNum(); }
    pfvDispPipe* getPipe(int i) { return (pfvDispPipe*)(listPipes->get(i)); }
    pfvDispPipe* findPipe(char* _name){
	return (pfvDispPipe*)pfvObject::find(listPipes,_name); 
    }

 	
    // views

    virtual pfvDispView* createView(pfvXmlNode*xml=NULL);
	
    int getNumViews() { return listViews->getNum(); }
    pfvDispView* getView(int i) { return (pfvDispView*)(listViews->get(i)); }
    pfvDispView* findView(char* _name){
    	return (pfvDispView*)pfvObject::find(listViews,_name); 
    }

    // pwins
	
    int getNumPWins() { return listPWins->getNum(); }
    pfvDispPWin* getPWin(int i) { return (pfvDispPWin*)(listPWins->get(i)); }
    pfvDispPWin* findPWin(char* _name){
	return (pfvDispPWin*)pfvObject::find(listPWins,_name);
    }
    pfvDispPWin* findPWin(pfPipeWindow* pw);


    // chans

    int getNumChans() { return listChans->getNum(); }
    pfvDispChan* getChan(int i) { return (pfvDispChan*)(listChans->get(i)); }
    pfvDispChan* findChan(char* _name){
	return (pfvDispChan*)pfvObject::find(listChans,_name);
    }

	
protected:

    friend class pfvDispPWin;
    friend class pfvDispChan;

    static pfType *classType;

    static pfvDisplayMngr* mngr;

    virtual int addPWin(pfvDispPWin* pw);
    virtual int addChan(pfvDispChan* chan);
    
    pfList* listPipes;
    pfList* listViews;
    pfList* listPWins;
    pfList* listChans;
};


///////////////////////////////////////////////////////////////////////////////


class PFV_DLLEXPORT pfvDispPipe : public pfvObject
{
protected:	

friend class pfvDisplayMngr;

    pfvDispPipe(); 
    pfvDispPipe(pfvXmlNode* xml);
    pfvDispPipe(int _screen);	
    ~pfvDispPipe();
    void construct();

    virtual int setXmlField(pfvXmlNode*xml);

public:
    static void init();
    static pfType* getClassType() { return classType; }
    virtual const char* getTypeName() const { return "pfvDispPipe"; }

    virtual int config();

    pfPipe* getHandle() { return pfpipe; }

    virtual pfvDispPWin* createPWin(pfvXmlNode* xml=NULL);

    int getNumPWins(){ return listPWins->getNum(); }
    pfvDispPWin* getPWin(int i) { return (pfvDispPWin*)(listPWins->get(i)); }
    pfvDispPWin* findPWin(char* _name){
	return (pfvDispPWin*)pfvObject::find(listPWins,_name); 
    }

    int setWSConnName(char* _name);
    char* getWSConnName() { return WSConnName; }
    
    int setScreen(int _screen);
    int getScreen() { return screen; }
	
	
protected:
	
friend class pfvDispPWin;

    static pfType *classType;

    pfPipe* pfpipe;

    pfList* listPWins;

    int screen;
    char WSConnName[256];
    
};


///////////////////////////////////////////////////////////////////////////////


class PFV_DLLEXPORT pfvDispPWin : public pfvObject
{
protected:

friend class pfvDispPipe;
friend class pfvDisplayMngr;

    pfvDispPWin(int pipe=0);
    pfvDispPWin(int pipe, pfvXmlNode* xml);
    ~pfvDispPWin();
    void construct(int p);

    virtual int setXmlField(pfvXmlNode* root);

public:

    static void init();
    static pfType* getClassType() { return classType; }
    virtual const char* getTypeName(void) const { return "pfvDispPWin"; }

    virtual int config();

    pfPipeWindow* getHandle() { return pfpwin; }
 
    pfvDispPipe* getPipe() { return pipe; }

    virtual pfvDispChan* createChan(pfvXmlNode* xml=NULL);
    int getNumChans() { return listChans->getNum(); }    
    pfvDispChan* getChan(int i) { return (pfvDispChan*)(listChans->get(i)); }

    virtual void setTitle(char* _title);
    char* getTitle() { return title? title : (char*)""; }
	
    virtual void setFullScreen(int v);
    int getFullScreen() { return fullScreen; }
    
    virtual void setOrigin(int x, int y);
    void getOrigin(int* x, int* y){ if(x) *x = origX; if(y) *y = origY; }
    
    virtual void setSize(int w, int h);
    void getSize(int* w, int* h){ if(w)*w=width; if(h)*h=height; }

    void updateOrigSize();

    virtual void setBorder(int b);
    int getBorder() { return border; }

    virtual void setVisualId(int id) { visualId = id; }
    int getVisualId() { return visualId; }

    virtual void setFBConfigAttrs(int* attr);
    virtual int* getFBConfigAttrs(){ return FBAttrs; }

    virtual int setManaged(int mngd) { isManaged=mngd; return 1; }
    virtual int getManaged() { return isManaged; }

protected:

friend class pfvDispChan;
friend void pfDispPWIN_OpenXWin(pfPipeWindow* pfpwin);

    void parseFBConfigTag(pfvXmlNode* xml);
    void openXWin();
	
    static pfType *classType;

    pfPipeWindow *pfpwin;

    int isManaged;

    pfvDispPipe *pipe;
    pfList* listChans;
	
    int fullScreen;
    int origX, origY, width, height;
    int border;
    char* title;

    pfuGLXWindow*glxwin;
    int visualId;
    int FBAttrs[64];
};
	
///////////////////////////////////////////////////////////////////////////////


class PFV_DLLEXPORT pfvDispChan : public pfvObject
{
protected:

friend class pfvDispPWin;
friend class pfvDisplayMngr;

    pfvDispChan(pfvDispPWin* pwin);
    pfvDispChan(pfvDispPWin* pwin, pfvXmlNode* xml);
    ~pfvDispChan();
    void construct(pfvDispPWin* _pwin);

    virtual int setXmlField(pfvXmlNode* xml);
    
public:

    static void init();
    static pfType* getClassType() { return classType; }
    virtual const char* getTypeName(void) const { return "pfvDispChan"; }


    virtual int config();

    pfChannel* getHandle() { return pfchan; }
	
    pfvDispPWin* getPWin() { return pwin; }
	
    pfvDispView* getView() { return dispview; }
	
    virtual void setFullWindow(int v);
    int getFullWindow() { return fullWindow; }

    virtual void setViewport(float l, float r, float b, float t);
    void getViewport(float* l, float* r, float* b, float* t){   
	if(l) *l=viewport[0]; if(r) *r=viewport[1]; 
        if(b) *b=viewport[2]; if(t) *t=viewport[3]; 
    }

    virtual void setFOV(float x, float y);
    virtual void setFOV(float l, float r, float b, float t);

    virtual void setOrtho(float l, float r, float b, float t);

    virtual int getFOV(float* x, float* y){   
	if(frustType==PFFRUST_ORTHOGONAL)
	    return 0;
	if(x) *x = fov[0] - fov[1];
        if(y) *y = fov[2] - fov[3];
	return 1;
    }
	
    virtual int getFOV(float* l, float* r, float* b, float* t){
	if(frustType==PFFRUST_ORTHOGONAL)
	    return 0;
	if(l) *l=fov[0]; if(r) *r=fov[1]; 
        if(b) *b=fov[2]; if(t) *t=fov[3];
	return 1;
    }

    virtual int getOrtho(float* l, float* r, float* b, float* t){
	if(frustType!=PFFRUST_ORTHOGONAL)
	    return 0;
	if(l) *l=fov[0]; if(r) *r=fov[1]; 
        if(b) *b=fov[2]; if(t) *t=fov[3];
	return 1;
    }

    void setAutoAspect(int which);
    int getAutoAspect() { return autoAspect; }
	
    int getFrustType() { return frustType; }
    
    virtual void setXYZOffset(float x, float y, float z);
    void getXYZOffset(float* x, float* y, float* z){   
	if(x) *x = xyzOffset[0];
        if(y) *y = xyzOffset[1];
        if(z) *z = xyzOffset[2]; 
    }
	    
    virtual void setHPROffset( float h, float p, float r );
    void getHPROffset( float*h, float*p, float*r ){   
	if(h) *h = hprOffset[0]; 
        if(p) *p = hprOffset[1];
        if(r) *r = hprOffset[2]; 
    }


    void getViewRange(float* l, float* r, float* b, float* t){   
	if(l) *l=viewRange[0]; if(r) *r=viewRange[1]; 
	if(b) *b=viewRange[2]; if(t) *t=viewRange[3]; 
    }

    void setViewRange(float l, float r, float b, float t); 	
	
	
protected:

    static pfType *classType;

    pfChannel* pfchan;

    pfvDispPWin* pwin;
    pfvDispView* dispview;
 
    int fullWindow;
    float viewport[4];
    
    float fov[4];
    int frustType;
    int autoAspect;

    pfVec3 xyzOffset;
    pfVec3 hprOffset;
    
    float viewRange[4];

friend class pfvDispView;
    virtual int setView( pfvDispView* v );    
};

	
///////////////////////////////////////////////////////////////////////////////


class PFV_DLLEXPORT pfvDispView : public pfvObject
{
protected:

friend class pfvDisplayMngr;

    pfvDispView();
    pfvDispView(pfvXmlNode*xml);
    ~pfvDispView();
    void construct();

public:

    static void init();
    static pfType* getClassType() { return classType; }
    virtual const char* getTypeName(void) const { return "pfvDispView"; }


    virtual int config();

    virtual int setXmlField(pfvXmlNode* xml);
	
    int getNumChans() { return listChans ? listChans->getNum() : 0; }
    pfvDispChan* getChan(int i){ return (pfvDispChan*)(listChans->get(i)); }

    pfvDispChan* findChan(char* _name){
	return (pfvDispChan*)pfvObject::find(listChans,_name);
    }

    virtual int addChan(pfvDispChan* chan);

    uint64_t getChanMask() { return chanMask; }

    
    pfChannel* getMasterChan() { return masterChan; }
	
    virtual int setMasterChan(pfvDispChan* chan);
    virtual int setMasterChan(pfChannel* chan);
		    
    uint getChanShare() { return share; }
    virtual uint setChanShare(uint share);

    pfScene *getScene() { return scene; }
    virtual int setScene(pfScene* _scene);

    virtual void setEye(pfVec3& xyz, pfVec3& hpr);
    void getEye(pfVec3&xyz, pfVec3&hpr);
	
protected:
	
    static pfType *classType;

    pfScene* scene;

    pfList* listChans;
    pfChannel* masterChan;
    uint64_t chanMask;
    
    int share;

    pfCoord eye;

};

///////////////////////////////////////////////////////////////////////////////

#endif // end of __PFV_DISPLAY_H__

