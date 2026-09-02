//
// Copyright 1995, Silicon Graphics, Inc.
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
// pfPipeWindow.h		pfPipeWindow include file
//
// $Revision: 1.122 $
// $Date: 2002/11/02 00:12:42 $
//

#ifndef __PF_PIPEWINDOW_H__
#define __PF_PIPEWINDOW_H__

#ifndef PF_CPLUSPLUS_API
#define PF_CPLUSPLUS_API 1
#else
#if !PF_CPLUSPLUS_API
#error "Cannot include C++ API header with PF_CPLUSPLUS_API disabled."
#endif
#endif


#include <Performer/pf/pfUpdatable.h>
#include <Performer/pf/pfLists.h>
#include <Performer/pf/pfUpdate.h>
#include <Performer/pf/pfPipe.h>
#include <Performer/pr/pfWindow.h>


class pfBuffer;
class pfPipeVideoChannel;

// For request() and openStatus
#define PFPWIN_NONE	    0x0
#define PFPWIN_CONFIG	    0x1
#define PFPWIN_OPEN	    0x2
#define PFPWIN_CLOSE	    0x4
#define PFPWIN_CLOSE_GL	    0x8

struct _pfPipeWinUpBuffer;


extern "C" {     // EXPORT to CAPI
/* ------------------- pfPipeWindow Tokens ----------------------- */
/* pfWinType */

#define PFPWIN_TYPE_X	    	PFWIN_TYPE_X
#define PFPWIN_TYPE_STATS   	PFWIN_TYPE_STATS
#define PFPWIN_TYPE_UNMANAGED  	PFWIN_TYPE_UNMANAGED
#define PFPWIN_TYPE_PBUFFER  	PFWIN_TYPE_PBUFFER
#define PFPWIN_TYPE_SHARE   	0x10000
#define PFPWIN_TYPE_NOXEVENTS   0x20000
#define PFPWIN_TYPE_MASK    (PFWIN_TYPE_MASK | PFPWIN_TYPE_SHARE | PFPWIN_TYPE_NOXEVENTS)

/* ----------------pfPipeWindow Related Routines------------------ */
extern DLLEXPORT void		pfInitGfx(void);
} // extern "C" END of C include export


#define PFPWIN_SHARE_ALL	0xffff

#define PFPIPEWINDOW ((pfPipeWindow*)_pfCurrentBuffer->pf_indexUpdatable(this))

#define PFPIPEWINDOWBUFFER ((pfPipeWindow*)buf->pf_indexUpdatable(this))

class DLLEXPORT pfPipeWindow : public pfUpdatable
{
public:

    void setName(const char *name)  {
        PFPIPEWINDOW->nb_setName(name);
    }

    const char* getName()  {
        return PFPIPEWINDOW->nb_getName();
    }

    void setWSConnectionName(const char *name)  {
        PFPIPEWINDOW->nb_setWSConnectionName(name);
    }

    const char* getWSConnectionName()  {
        return PFPIPEWINDOW->nb_getWSConnectionName();
    }

    void setMode(int mode, int val)  {
        PFPIPEWINDOW->nb_setMode(mode, val);
    }

    int getMode(int _mode)  {
        return PFPIPEWINDOW->nb_getMode(_mode);
    }

    void setWinType(uint type)  {
        PFPIPEWINDOW->nb_setWinType(type);
    }

    uint getWinType() const  {
        return PFPIPEWINDOW->nb_getWinType();
    }

    pfState* getCurState()  {
        return PFPIPEWINDOW->nb_getCurState();
    }

    void setAspect(int x, int y)  {
        PFPIPEWINDOW->nb_setAspect(x, y);
    }

    void getAspect(int *x, int *y)  {
        PFPIPEWINDOW->nb_getAspect(x, y);
    }

    void setOriginSize(int xo, int yo, int xs, int ys)  {
        PFPIPEWINDOW->nb_setOriginSize(xo, yo, xs, ys);
    }

    void setOrigin(int xo, int yo)  {
        PFPIPEWINDOW->nb_setOrigin(xo, yo);
    }

    void getOrigin(int *xo, int *yo)  {
        PFPIPEWINDOW->nb_getOrigin(xo, yo);
    }

    void getScreenOrigin(int *xo, int *yo)  {
        PFPIPEWINDOW->nb_getScreenOrigin(xo, yo);
    }

    void setSize(int xs, int ys)  {
        PFPIPEWINDOW->nb_setSize(xs, ys);
    }

    void getSize(int *xs, int *ys)  {
        PFPIPEWINDOW->nb_getSize(xs, ys);
    }

    void setFullScreen()  {
        PFPIPEWINDOW->nb_setFullScreen();
    }

    void getCurOriginSize(int *xo, int *yo, int *xs, int *ys)  {
        PFPIPEWINDOW->nb_getCurOriginSize(xo, yo, xs, ys);
    }

    void getCurScreenOriginSize(int *xo, int *yo, int *xs, int *ys)  {
        PFPIPEWINDOW->nb_getCurScreenOriginSize(xo, yo, xs, ys);
    }

    void setOverlayWin(pfWindow *ow)  {
        PFPIPEWINDOW->nb_setOverlayWin(ow);
    }

    pfWindow* getOverlayWin()  {
        return PFPIPEWINDOW->nb_getOverlayWin();
    }

    void setStatsWin(pfWindow *sw)  {
        PFPIPEWINDOW->nb_setStatsWin(sw);
    }

    pfWindow* getStatsWin()  {
        return PFPIPEWINDOW->nb_getStatsWin();
    }

    void setScreen(int screen)  {
        PFPIPEWINDOW->nb_setScreen(screen);
    }

    int getScreen()  {
        return PFPIPEWINDOW->nb_getScreen();
    }

    void setShare(int mode)  {
        PFPIPEWINDOW->nb_setShare(mode);
    }

    uint getShare()  {
        return PFPIPEWINDOW->nb_getShare();
    }

    void setWSWindow(pfWSConnection dsp, pfWSWindow wsw)  {
        PFPIPEWINDOW->nb_setWSWindow(dsp, wsw);
    }

    Window getWSWindow()  {
        return PFPIPEWINDOW->nb_getWSWindow();
    }

    void setWSDrawable(pfWSConnection dsp, pfWSDrawable gxw)  {
        PFPIPEWINDOW->nb_setWSDrawable(dsp, gxw);
    }

    pfWSDrawable getWSDrawable()  {
        return PFPIPEWINDOW->nb_getWSDrawable();
    }

    pfWSDrawable getCurWSDrawable()  {
        return PFPIPEWINDOW->nb_getCurWSDrawable();
    }

    void setFBConfigData(void *_data)  {
        PFPIPEWINDOW->nb_setFBConfigData(_data);
    }

    void* getFBConfigData()  {
        return PFPIPEWINDOW->nb_getFBConfigData();
    }

    void setFBConfigAttrs(int *attr)  {
        PFPIPEWINDOW->nb_setFBConfigAttrs(attr);
    }

    int* getFBConfigAttrs()  {
        return PFPIPEWINDOW->nb_getFBConfigAttrs();
    }

    void setFBConfig(pfGLXFBConfig fbc)  {
        PFPIPEWINDOW->nb_setFBConfig(fbc);
    }

    void setFBConfigId(int vId)  {
        PFPIPEWINDOW->nb_setFBConfigId(vId);
    }

    int getFBConfigId()  {
        return PFPIPEWINDOW->nb_getFBConfigId();
    }

    void setIndex(int index)  {
        PFPIPEWINDOW->nb_setIndex(index);
    }

    int getIndex()  {
        return PFPIPEWINDOW->nb_getIndex();
    }

    pfWindow* getSelect()  {
        return PFPIPEWINDOW->nb_getSelect();
    }

    void setGLCxt(pfGLContext gc)  {
        PFPIPEWINDOW->nb_setGLCxt(gc);
    }

    pfGLContext getGLCxt()  {
        return PFPIPEWINDOW->nb_getGLCxt();
    }

    void setSwapBarrier(int _barrierName)  {
        PFPIPEWINDOW->nb_setSwapBarrier(_barrierName);
    }

    int getSwapBarrier()  {
        return PFPIPEWINDOW->nb_getSwapBarrier();
    }

    void setWinList(pfList *_wl)  {
        PFPIPEWINDOW->nb_setWinList(_wl);
    }

    pfList* getWinList() const  {
        return PFPIPEWINDOW->nb_getWinList();
    }

    int attachWin(pfWindow *w1)  {
        return PFPIPEWINDOW->nb_attachWin(w1);
    }

    int detachWin(pfWindow *w1)  {
        return PFPIPEWINDOW->nb_detachWin(w1);
    }

    int attach(pfPipeWindow *pw1)  {
        return PFPIPEWINDOW->nb_attach(pw1);
    }

    int detach(pfPipeWindow *pw1)  {
        return PFPIPEWINDOW->nb_detach(pw1);
    }

    void attachSwapGroup(pfPipeWindow *_w1)  {
        PFPIPEWINDOW->nb_attachSwapGroup(_w1);
    }

    void attachWinSwapGroup(pfWindow *_w1)  {
        PFPIPEWINDOW->nb_attachWinSwapGroup(_w1);
    }

    void detachSwapGroup()  {
        PFPIPEWINDOW->nb_detachSwapGroup();
    }

    pfWindow* select()  {
        return PFPIPEWINDOW->nb_select();
    }

    void swapBuffers()  {
        PFPIPEWINDOW->nb_swapBuffers();
    }

    pfFBConfig chooseFBConfig(int *attr)  {
        return PFPIPEWINDOW->nb_chooseFBConfig(attr);
    }

    int isOpen()  {
        return PFPIPEWINDOW->nb_isOpen();
    }

    int isManaged()  {
        return PFPIPEWINDOW->nb_isManaged();
    }

    int inSwapGroup()  {
        return PFPIPEWINDOW->nb_inSwapGroup();
    }

    int query(int _which, int *_dst)  {
        return PFPIPEWINDOW->nb_query(_which, _dst);
    }

    int mQuery(int *_which, int *_dst)  {
        return PFPIPEWINDOW->nb_mQuery(_which, _dst);
    }

    pfPipe* getPipe()  {
        return PFPIPEWINDOW->nb_getPipe();
    }

    int getPipeIndex() const  {
        return PFPIPEWINDOW->nb_getPipeIndex();
    }

    void setConfigFunc(pfPWinFuncType _func)  {
        PFPIPEWINDOW->nb_setConfigFunc(_func);
    }

    pfPWinFuncType getConfigFunc()  {
        return PFPIPEWINDOW->nb_getConfigFunc();
    }

    int getChanIndex(pfChannel *_chan)  {
        return PFPIPEWINDOW->nb_getChanIndex(_chan);
    }

    void config()  {
        PFPIPEWINDOW->nb_config();
    }

    void open()  {
        PFPIPEWINDOW->nb_open();
    }

    void close()  {
        PFPIPEWINDOW->nb_close();
    }

    void closeGL()  {
        PFPIPEWINDOW->nb_closeGL();
    }

    int removeChan(pfChannel *_chan)  {
        return PFPIPEWINDOW->nb_removeChan(_chan);
    }

    int addChan(pfChannel *_chan)  {
        return PFPIPEWINDOW->nb_addChan(_chan);
    }

    void insertChan(int _where, pfChannel *_chan)  {
        PFPIPEWINDOW->nb_insertChan(_where, _chan);
    }

    int moveChan(int _where, pfChannel *_chan)  {
        return PFPIPEWINDOW->nb_moveChan(_where, _chan);
    }

    pfChannel* getChan(int which)  {
        return PFPIPEWINDOW->nb_getChan(which);
    }

    int getNumChans() const  {
        return PFPIPEWINDOW->nb_getNumChans();
    }

    int addPVChan(pfPipeVideoChannel *_vchan)  {
        return PFPIPEWINDOW->nb_addPVChan(_vchan);
    }

    void setPVChan(int _num, pfPipeVideoChannel *_vchan)  {
        PFPIPEWINDOW->nb_setPVChan(_num, _vchan);
    }

    void removePVChan(pfPipeVideoChannel *_vchan)  {
        PFPIPEWINDOW->nb_removePVChan(_vchan);
    }

    void removePVChanIndex(int _num)  {
        PFPIPEWINDOW->nb_removePVChanIndex(_num);
    }

    pfPipeVideoChannel* getPVChan(int _num)  {
        return PFPIPEWINDOW->nb_getPVChan(_num);
    }

    pfPipeVideoChannel* getPVChanId(int _num)  {
        return PFPIPEWINDOW->nb_getPVChanId(_num);
    }

    pfPipeVideoChannel* getPosPVChan(int _x, int _y)  {
        return PFPIPEWINDOW->nb_getPosPVChan(_x, _y);
    }

    int getPVChanIndex(pfPipeVideoChannel *_vchan)  {
        return PFPIPEWINDOW->nb_getPVChanIndex(_vchan);
    }

    int getNumPVChans() const  {
        return PFPIPEWINDOW->nb_getNumPVChans();
    }

    void bindPVChans()  {
        PFPIPEWINDOW->nb_bindPVChans();
    }

    void unbindPVChans()  {
        PFPIPEWINDOW->nb_unbindPVChans();
    }
public:		// Constructors/Destructors
    //CAPI:basename PWin
    //CAPI:updatable
    //CAPI:newargs
    pfPipeWindow(pfPipe *p);
    virtual ~pfPipeWindow();

protected:
    friend void _pfCalcVideoRate(void);
    
    // Create a PipeWindow 
    pfPipeWindow(pfBuffer *buf, pfPipe *parent);
    // Clone a new PipeWindow in buffer.
    pfPipeWindow(const pfPipeWindow* pw, pfBuffer *buf);

public:
    // per class functions;
    static void	   init();
    static pfType* getClassType() { return classType; }
 
public:			// pfMemory virtual functions
    virtual int		 print(uint _travMode, uint _verbose, char *_prefix, FILE *_file);
    void	 	printSwapGroup() { prWin -> printSwapGroup(); }

PFINTERNAL:		// pfUpdatable virtual functions
    virtual void	pf_copyToDraw(const pfUpdatable *_srcPWin); 
    // Update pfPipeWindow.
    virtual void	pf_applyUpdate(const pfUpdatable *prev, int  updateId);
    // Create clone of pfPipeWindow in buffer buf
    virtual pfUpdatable* pf_bufferClone(pfBuffer *buf);
    virtual void    	pf_updateBufferList(pfBuffer *buf, int addRemove) {
        buf->pf_updateList(this, addRemove);
    }
    int			 pf_print(uint _travMode, uint _verbose, char *_prefix, FILE *_file); 

PFINTERNAL:
    // pfWindow set/get lookalike API
    void	nb_setName(const char *name);
    const char*	nb_getName() {
	return prWin->getName();
    }

    void	nb_setWSConnectionName(const char *name);
    const char*	nb_getWSConnectionName() {
	return prWin->getWSConnectionName();
    }

    void	nb_setMode(int mode, int val);
    int		nb_getMode(int _mode) {
	return prWin->getMode(_mode); 	
    }

    // CAPI:verb PWinType
    void	nb_setWinType(uint type);
    // CAPI:verb GetPWinType
    uint	nb_getWinType() const;

    // CAPI:noverb

    pfState*	nb_getCurState() {
	return prWin->getCurState();
    }
    void	nb_setAspect(int x, int y);
    void	nb_getAspect(int *x, int *y) {
	prWin->getAspect(x, y); 
    }

    void	nb_setOriginSize(int xo, int yo, int xs, int ys);
    void	nb_setOrigin(int xo, int yo);
    void	nb_getOrigin(int *xo, int *yo);
    void	nb_getScreenOrigin(int *xo, int *yo);
    void	nb_setSize(int xs, int ys);
    void	nb_getSize(int *xs, int *ys);

    //CAPI:verb PWinFullScreen
    void	nb_setFullScreen();
    //CAPI:noverb

    void	nb_getCurOriginSize(int *xo, int *yo, int *xs, int *ys) {
	prWin->getCurOriginSize(xo, yo, xs, ys); 
    }
    void	nb_getCurScreenOriginSize(int *xo, int *yo, int *xs, int *ys) {
	prWin->getCurScreenOriginSize(xo, yo, xs, ys); 
    }

    void	nb_setOverlayWin(pfWindow *ow);
    pfWindow*	nb_getOverlayWin(void)  {
	return prWin->getOverlayWin();
    }

    void	nb_setStatsWin(pfWindow *sw);
    pfWindow*	nb_getStatsWin() {
	return prWin->getStatsWin();
    }

    void	nb_setScreen(int screen);
    int		nb_getScreen(void) {
	return prWin->getScreen();
    }

    void	nb_setShare(int mode);
    uint	nb_getShare(void) {
	return (shareMode & PFPWIN_SHARE_ALL);
    }

    void	nb_setWSWindow(pfWSConnection dsp, pfWSWindow wsw);
    Window	nb_getWSWindow(void) { return wsWin; }

    void	nb_setWSDrawable(pfWSConnection dsp, pfWSDrawable gxw);
    
    pfWSDrawable nb_getWSDrawable(void) {
      return prWin->getWSDrawable();
    }
    
    pfWSDrawable nb_getCurWSDrawable(void) {
      pfWindow *sw = prWin->getSelect();
      return sw->getWSDrawable();
    }

    void                nb_setFBConfigData(void *_data);
    void*               nb_getFBConfigData() {
	return prWin->getFBConfigData();
    }

    void	nb_setFBConfigAttrs(int *attr);
    int*	nb_getFBConfigAttrs() {
	return prWin->getFBConfigAttrs();
    }

    //CAPI:private
    void	nb_setFBConfig(pfGLXFBConfig fbc);
    //CAPI:public
    
    void	nb_setFBConfigId(int vId);
    int		nb_getFBConfigId() {
	return prWin->getFBConfigId();
    }

    void	nb_setIndex(int index);
    int		nb_getIndex() {
	return prWin->getIndex();
    }

    pfWindow*	nb_getSelect(void) {
	return prWin->getSelect();
    }

    void	nb_setGLCxt(pfGLContext gc);
    pfGLContext	nb_getGLCxt(void)  {
	return prWin->getGLCxt();
    }

    void	nb_setSwapBarrier(int _barrierName);
    int		nb_getSwapBarrier() { return prWin->getSwapBarrier(); }
    
    //CAPI:verb PWinList
    void	nb_setWinList(pfList *_wl);
    //CAPI:verb GetPWinList
    pfList*	nb_getWinList() const { 
	return prWin->getWinList();
    }

PFINTERNAL:		// pfWindow lookalike API functions

    // CAPI:verb
    //CAPI:verb AttachPWinWin
    int		nb_attachWin(pfWindow *w1);
    //CAPI:verb DetachPWinWin
    int		nb_detachWin(pfWindow *w1);
    int		nb_attach(pfPipeWindow *pw1);
    int		nb_detach(pfPipeWindow *pw1);
    
    /* swap group */
    //CAPI:verb AttachPWinSwapGroup
    void	nb_attachSwapGroup(pfPipeWindow *_w1);
    //CAPI:verb AttachPWinWinSwapGroup
    void	nb_attachWinSwapGroup(pfWindow *_w1);
    //CAPI:verb DetachPWinSwapGroup
    void	nb_detachSwapGroup();
    
    pfWindow*	nb_select();

    // CAPI:verb SwapPWinBuffers
    void	nb_swapBuffers() {
	prWin->swapBuffers();
    }

    // CAPI:verb ChoosePWinFBConfig
    pfFBConfig	nb_chooseFBConfig(int *attr);

    // CAPI:verb IsPWinOpen
    int		nb_isOpen() {
	return prWin->isOpen();
    }
    int		nb_isManaged() {
	return prWin->isManaged();
    }
    // CAPI:verb PWinInSwapGroup
    int		nb_inSwapGroup()  { return prWin->inSwapGroup(); }

    int		nb_query(int _which, int *_dst) {
	return prWin->query(_which, _dst);
    }

    int		nb_mQuery(int *_which, int *_dst) {
	return prWin->mQuery(_which, _dst);
    }

PFINTERNAL:		// class specific sets/gets
    pfPipe*	nb_getPipe();
    int		nb_getPipeIndex(void) const {
	return pipeIndex;
    }

    void	nb_setConfigFunc(pfPWinFuncType _func);
    pfPWinFuncType   nb_getConfigFunc() {
	return configFunc;
    }
    int		nb_getChanIndex(pfChannel *_chan);

PFINTERNAL:		// class specific public functions 
    // these queue up a request
    //CAPI:verb
    void	nb_config();
    void	nb_open();
    void	nb_close();
    //CAPI:verb ClosePWinGL
    void	nb_closeGL();

    // pfChannel list management
    //CAPI:noverb
    //CAPI:basename
    //CAPI:updatable
    int		nb_removeChan(pfChannel *_chan);
    int		nb_addChan(pfChannel *_chan);
    void	nb_insertChan(int _where, pfChannel *_chan);
    int		nb_moveChan(int _where, pfChannel *_chan);
    pfChannel*	nb_getChan(int which);
    int		nb_getNumChans(void) const {
	return chanList.getNum();
    }
    
    //CAPI:basename PWin
    
    // pfPipeVideoChannel list management
    int		nb_addPVChan(pfPipeVideoChannel *_vchan);
    void	nb_setPVChan(int _num, pfPipeVideoChannel *_vchan);
    void	nb_removePVChan(pfPipeVideoChannel *_vchan);
    void	nb_removePVChanIndex(int _num);


    pfPipeVideoChannel *nb_getPVChan(int _num);
    pfPipeVideoChannel *nb_getPVChanId(int _num);
    pfPipeVideoChannel *nb_getPosPVChan(int _x, int _y);

    int nb_getPVChanIndex(pfPipeVideoChannel *_vchan);
    int nb_getNumPVChans(void) const {
	return pVChanList.getNum();
    }
    
    /* pfPipeVideoChannel Management */
    //CAPI:verb BindPWinPVChans
    void	nb_bindPVChans();
    //CAPI:verb UnbindPWinPVChans
    void	nb_unbindPVChans();
    


private:
    int		    	pipeIndex;
    int			winType, userWinType, shareMode, dirty, prevMod;
    int		    	newXSize, newYSize;
    int			mpConfig;
    int		    	mpDrawCount;
    int			pvcInited;
    
    // Indicates whether pfWindow is opened or not; status is framestamped
    int			*openStatus, *openStatusFrame;

    // Request to change status of pfWindow made from APP; request is framestamped
    int			openRequest, openRequestFrame, isOpened;

    // used for catching events
    pfWSWindow	    	wsWin, eventWin, drawWin; 

    // libpr pfWindow structure
    pfWindow	    	*prWin;	

    // pfPipe which renders into this window
    _pfPipeRef	    	parentPipe;

    // List of channels which render into this window
    _pfChannelList   	chanList;
    
    // List of video channels bound to this window
    _pfPipeVideoChannelList pVChanList;

    // Buffer for locked copies upstream to the APP
    _pfPipeWinUpBuffer	*upstreamBuffer;

    // Window initialization function called after window is opened
    pfPWinFuncType 	configFunc;

private:
    // lists can hold pfPipeWindows and pfWindows
    // lists hold ptrs to APP copies so that lists can be compared
    pfList		*shareGroupList; 
    pfList		*swapGroupList; 
    static pfType *classType;
    // The wndclass that pipewindows use
    static WNDCLASS dftWndClass;
public:
    void *_pfPipeWindowExtraData;
};

PF_UPDATABLE_REF(pfPipeWindow, _pfPipeWindowRef);

#endif // !__PF_PIPEWINDOW_H__
